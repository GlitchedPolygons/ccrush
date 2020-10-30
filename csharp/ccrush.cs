using System;
using System.IO;
using System.Reflection;
using System.Text;
using System.Runtime.InteropServices;

namespace GlitchedPolygons.CcrushSharp
{
    /// <summary>
    /// CcrushSharp class that wraps the native C functions from the ccrush library. <para> </para>
    /// Copy this class into your own C# project and then don't forget to
    /// copy the lib/ folder to your own project's build output directory!
    /// </summary>
    public class CcrushSharpContext : IDisposable
    {
        #region Shared library loaders (per platform implementations)

        private interface ISharedLibLoadUtils
        {
            IntPtr LoadLibrary(string fileName);
            void FreeLibrary(IntPtr handle);
            IntPtr GetProcAddress(IntPtr handle, string name);
        }

        private class SharedLibLoadUtilsWindows : ISharedLibLoadUtils
        {
            [DllImport("kernel32.dll")]
            private static extern IntPtr LoadLibrary(string fileName);

            [DllImport("kernel32.dll")]
            private static extern int FreeLibrary(IntPtr handle);

            [DllImport("kernel32.dll")]
            private static extern IntPtr GetProcAddress(IntPtr handle, string procedureName);

            void ISharedLibLoadUtils.FreeLibrary(IntPtr handle)
            {
                FreeLibrary(handle);
            }

            IntPtr ISharedLibLoadUtils.GetProcAddress(IntPtr dllHandle, string name)
            {
                return GetProcAddress(dllHandle, name);
            }

            IntPtr ISharedLibLoadUtils.LoadLibrary(string fileName)
            {
                return LoadLibrary(fileName);
            }
        }

        private class SharedLibLoadUtilsLinux : ISharedLibLoadUtils
        {
            const int RTLD_NOW = 2;

            [DllImport("libdl.so")]
            private static extern IntPtr dlopen(String fileName, int flags);

            [DllImport("libdl.so")]
            private static extern IntPtr dlsym(IntPtr handle, String symbol);

            [DllImport("libdl.so")]
            private static extern int dlclose(IntPtr handle);

            [DllImport("libdl.so")]
            private static extern IntPtr dlerror();

            public IntPtr LoadLibrary(string fileName)
            {
                return dlopen(fileName, RTLD_NOW);
            }

            public void FreeLibrary(IntPtr handle)
            {
                dlclose(handle);
            }

            public IntPtr GetProcAddress(IntPtr dllHandle, string name)
            {
                dlerror();
                IntPtr res = dlsym(dllHandle, name);
                IntPtr err = dlerror();
                if (err != IntPtr.Zero)
                {
                    throw new Exception("dlsym: " + Marshal.PtrToStringAnsi(err));
                }

                return res;
            }
        }

        private class SharedLibLoadUtilsMac : ISharedLibLoadUtils
        {
            const int RTLD_NOW = 2;

            [DllImport("libdl.dylib")]
            private static extern IntPtr dlopen(String fileName, int flags);

            [DllImport("libdl.dylib")]
            private static extern IntPtr dlsym(IntPtr handle, String symbol);

            [DllImport("libdl.dylib")]
            private static extern int dlclose(IntPtr handle);

            [DllImport("libdl.dylib")]
            private static extern IntPtr dlerror();

            public IntPtr LoadLibrary(string fileName)
            {
                return dlopen(fileName, RTLD_NOW);
            }

            public void FreeLibrary(IntPtr handle)
            {
                dlclose(handle);
            }

            public IntPtr GetProcAddress(IntPtr dllHandle, string name)
            {
                dlerror();
                IntPtr res = dlsym(dllHandle, name);
                IntPtr err = dlerror();
                if (err != IntPtr.Zero)
                {
                    throw new Exception("dlsym: " + Marshal.PtrToStringAnsi(err));
                }

                return res;
            }
        }

        #endregion

        #region Function mapping

        private delegate int CompressDelegate(
            [MarshalAs(UnmanagedType.LPArray)] byte[] data,
            [MarshalAs(UnmanagedType.U8)] ulong dataLength,
            [MarshalAs(UnmanagedType.U4)] uint bufferSizeKiB,
            [MarshalAs(UnmanagedType.I4)] int level,
            out IntPtr output,
            out ulong outputLength
        );

        private delegate int DecompressDelegate(
            [MarshalAs(UnmanagedType.LPArray)] byte[] data,
            [MarshalAs(UnmanagedType.U8)] ulong dataLength,
            [MarshalAs(UnmanagedType.U4)] uint bufferSizeKiB,
            out IntPtr output,
            out ulong outputLength
        );

        private delegate void FreeDelegate(IntPtr mem);

        private delegate uint GetVersionNumberDelegate();

        private delegate IntPtr GetVersionNumberStringDelegate();

        private CompressDelegate compressDelegate;
        private DecompressDelegate decompressDelegate;
        private FreeDelegate freeDelegate;

        #endregion

        private IntPtr lib;
        private ISharedLibLoadUtils loadUtils = null;

        /// <summary>
        /// Absolute path to the ccrush shared library that is currently loaded into memory for CcrushSharp.
        /// </summary>
        public string LoadedLibraryPath { get; }

        /// <summary>
        /// Current ccrush version.
        /// </summary>
        public uint Version { get; }

        /// <summary>
        /// Nicely formatted, and 100% human-readable ccrush version number string.
        /// </summary>
        public string VersionString { get; }

        /// <summary>
        /// Creates a new ccrush# instance. <para> </para>
        /// Make sure to create one only once and cache it as needed, since loading the DLLs into memory can negatively affect the performance.
        /// <param name="sharedLibPathOverride">[OPTIONAL] Don't look for a <c>lib/</c> folder and directly use this path as a pre-resolved, platform-specific shared lib/DLL file path. Pass this if you want to handle the various platform's paths yourself.</param>
        /// </summary>
        public CcrushSharpContext(string sharedLibPathOverride = null)
        {
            string os;
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                os = "windows";
                loadUtils = new SharedLibLoadUtilsWindows();
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
            {
                os = "linux";
                loadUtils = new SharedLibLoadUtilsLinux();
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                os = "mac";
                loadUtils = new SharedLibLoadUtilsMac();
            }
            else
            {
                throw new PlatformNotSupportedException("Unsupported OS");
            }

            if (!string.IsNullOrEmpty(sharedLibPathOverride))
            {
                LoadedLibraryPath = sharedLibPathOverride;
            }
            else
            {
                string cpu;
                
                switch (RuntimeInformation.ProcessArchitecture)
                {
                    case Architecture.X64:
                        cpu = "x64";
                        break;
                    case Architecture.X86:
                        cpu = "x86";
                        break;
                    case Architecture.Arm:
                        cpu = "armeabi-v7a";
                        break;
                    case Architecture.Arm64:
                        cpu = "arm64-v8a";
                        break;
                    default:
                        throw new PlatformNotSupportedException("CPU Architecture not supported!");
                }

                string path = Path.Combine(Path.GetFullPath(Path.GetDirectoryName(Assembly.GetCallingAssembly().Location) ?? "."), "lib", cpu, os);

                if (!Directory.Exists(path))
                {
                    throw new PlatformNotSupportedException($"Shared library not found in {path} and/or unsupported CPU architecture. Please don't forget to copy the shared libraries/DLL into the 'lib/{{CPU_ARCHITECTURE}}/{{OS}}/{{SHARED_LIB_FILE}}' folder of your output build directory. ");
                }

                bool found = false;
                foreach (string file in Directory.GetFiles(path))
                {
                    if (file.ToLower().Contains("ccrush"))
                    {
                        LoadedLibraryPath = Path.GetFullPath(Path.Combine(path, file));
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    throw new FileLoadException($"Shared library not found in {path} and/or unsupported CPU architecture. Please don't forget to copy the shared libraries/DLL into the 'lib/{{CPU_ARCHITECTURE}}/{{OS}}/{{SHARED_LIB_FILE}}' folder of your output build directory. ");
                }
            }

            lib = loadUtils.LoadLibrary(LoadedLibraryPath);
            if (lib == IntPtr.Zero)
            {
                goto hell;
            }

            IntPtr compress = loadUtils.GetProcAddress(lib, "ccrush_compress");
            if (compress == IntPtr.Zero)
            {
                goto hell;
            }

            IntPtr decompress = loadUtils.GetProcAddress(lib, "ccrush_decompress");
            if (decompress == IntPtr.Zero)
            {
                goto hell;
            }

            IntPtr free = loadUtils.GetProcAddress(lib, "ccrush_free");
            if (free == IntPtr.Zero)
            {
                goto hell;
            }

            IntPtr getVersionNumber = loadUtils.GetProcAddress(lib, "ccrush_get_version_nr");
            if (getVersionNumber == IntPtr.Zero)
            {
                goto hell;
            }

            IntPtr getVersionNumberString = loadUtils.GetProcAddress(lib, "ccrush_get_version_nr_string");
            if (getVersionNumberString == IntPtr.Zero)
            {
                goto hell;
            }

            compressDelegate = Marshal.GetDelegateForFunctionPointer<CompressDelegate>(compress);
            decompressDelegate = Marshal.GetDelegateForFunctionPointer<DecompressDelegate>(decompress);
            freeDelegate = Marshal.GetDelegateForFunctionPointer<FreeDelegate>(free);

            var getVersionNumberDelegate = Marshal.GetDelegateForFunctionPointer<GetVersionNumberDelegate>(getVersionNumber);
            var getVersionNumberStringDelegate = Marshal.GetDelegateForFunctionPointer<GetVersionNumberStringDelegate>(getVersionNumberString);

            Version = getVersionNumberDelegate.Invoke();
            VersionString = Marshal.PtrToStringAnsi(getVersionNumberStringDelegate.Invoke());

            return;

            hell:
            throw new Exception($"Failed to load one or more functions from the shared library \"{LoadedLibraryPath}\"!");
        }

        private static byte[] MarshalReadBytes(IntPtr array, ulong arrayLength, uint bufferSizeKiB = 256)
        {
            using (var ms = new MemoryStream((int)arrayLength))
            {
                IntPtr i = array;
                ulong rem = arrayLength;
                byte[] buf = new byte[bufferSizeKiB * 1024];

                while (rem != 0)
                {
                    int n = (int)Math.Min(rem, (ulong)buf.LongLength);
                    Marshal.Copy(i, buf, 0, n);
                    i = IntPtr.Add(i, n);
                    rem -= (ulong)n;
                    ms.Write(buf, 0, n);
                }

                return ms.ToArray();
            }
        }

        /// <summary>
        /// Frees unmanaged resources (unloads the shared lib/dll).
        /// </summary>
        public void Dispose()
        {
            loadUtils.FreeLibrary(lib);
        }

        /// <summary>
        /// Compresses an array of bytes using deflate.
        /// </summary>
        /// <param name="data">The data to compress.</param>
        /// <param name="level">The level of compression <c>[0-9]</c>. Lower means faster, higher level means better compression (but slower). Default is <c>6</c>. If you pass a value that is out of the allowed range of <c>[0-9]</c>, <c>6</c> will be used! <c>0</c> does not compress at all...</param>
        /// <param name="bufferSizeKiB"></param>
        /// <returns>The compressed data.</returns>
        public byte[] Compress(byte[] data, int level = 6, uint bufferSizeKiB = 256)
        {
            switch (compressDelegate(data, (ulong)data.LongLength, bufferSizeKiB, level, out IntPtr output, out ulong outputLength))
            {
                case 1000:
                    throw new ArgumentException("One or more arguments null or invalid!");
                case 2000:
                    throw new OutOfMemoryException();
                default:
                    byte[] o = MarshalReadBytes(output, outputLength, bufferSizeKiB);
                    freeDelegate(output);
                    return o;
            }
        }

        /// <summary>
        /// Decompresses a given set of deflated data using inflate.
        /// </summary>
        /// <param name="data">The compressed bytes to decompress.</param>
        /// <param name="bufferSizeKiB">The underlying buffer size to use (in KiB). If available, a buffer size of 256KiB or more is recommended.</param>
        /// <returns>The decompressed data.</returns>
        /// <exception cref="ArgumentException">Thrown when one or more arguments are <c>null</c> and/or invalid.</exception>
        /// <exception cref="OutOfMemoryException">Thrown when the gates of hell opened.</exception>
        public byte[] Decompress(byte[] data, uint bufferSizeKiB = 256)
        {
            switch (decompressDelegate(data, (ulong)data.LongLength, bufferSizeKiB, out IntPtr output, out ulong outputLength))
            {
                case 1000:
                    throw new ArgumentException("One or more arguments null or invalid!");
                case 2000:
                    throw new OutOfMemoryException();
                default:
                    byte[] o = MarshalReadBytes(output, outputLength, bufferSizeKiB);
                    freeDelegate(output);
                    return o;
            }
        }
    }

    //  --------------------------------------------------------------------
    //  ------------------------------> DEMO <------------------------------
    //  --------------------------------------------------------------------

    internal static class Example
    {
        // DEMO
        // This is an example Main method that shows how the two CcrushSharp wrapper methods can be used.
        // Don't forget to copy the lib/ folder into your output build directory, otherwise CcrushSharp doesn't know from where to load the DLL/shared lib!

        private static void Main(string[] args)
        {
            var ccrush = new CcrushSharpContext();

            Console.WriteLine();
            Console.WriteLine(string.Format("CCRUSH VERSION: {0} ({1})", ccrush.VersionString, ccrush.Version));
            Console.WriteLine();

            const string text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. " +
                                "Orci sagittis eu volutpat odio facilisis mauris sit amet massa. Ut tortor pretium viverra suspendisse potenti nullam ac tortor vitae. " +
                                "Tempor id eu nisl nunc mi. Ut diam quam nulla porttitor massa id neque aliquam vestibulum. Nec nam aliquam sem et. Sed augue lacus viverra vitae. " +
                                "Suscipit tellus mauris a diam. Nunc lobortis mattis aliquam faucibus purus in. Lacus suspendisse faucibus interdum posuere. " +
                                "Elit eget gravida cum sociis natoque. Rhoncus mattis rhoncus urna neque viverra justo nec. Justo donec enim diam vulputate.\n\n" +
                                "Venenatis tellus in metus vulputate eu scelerisque felis imperdiet proin. Nec feugiat nisl pretium fusce id velit. Morbi non arcu risus quis varius quam. " +
                                "Viverra orci sagittis eu volutpat odio. Imperdiet dui accumsan sit amet nulla facilisi. Consectetur lorem donec massa sapien faucibus. " +
                                "Tortor dignissim convallis aenean et tortor. Elit scelerisque mauris pellentesque pulvinar. Cursus euismod quis viverra nibh cras. " +
                                "Est ultricies integer quis auctor elit sed vulputate mi sit. Sapien et ligula ullamcorper malesuada proin libero nunc. " +
                                "At augue eget arcu dictum varius duis at consectetur. Sapien pellentesque habitant morbi tristique senectus et netus et. " +
                                "Aliquet bibendum enim facilisis gravida neque. A scelerisque purus semper eget duis at tellus. Et pharetra pharetra massa massa ultricies mi quis.\n\n" +
                                "Senectus et netus et malesuada fames ac turpis egestas. Diam quam nulla porttitor massa id neque aliquam vestibulum. Est velit egestas dui id ornare arcu odio. " +
                                "Elementum eu facilisis sed odio morbi quis commodo odio. Posuere lorem ipsum dolor sit. Suscipit tellus mauris a diam. " +
                                "Hendrerit gravida rutrum quisque non tellus orci ac auctor. Aliquam etiam erat velit scelerisque in dictum non consectetur a. " +
                                "Aenean euismod elementum nisi quis eleifend quam adipiscing. Amet purus gravida quis blandit turpis. " +
                                "Ac placerat vestibulum lectus mauris ultrices eros in cursus turpis.\n\nFermentum leo vel orci porta non pulvinar neque. " +
                                "Fringilla ut morbi tincidunt augue interdum velit. Tristique nulla aliquet enim tortor at auctor urna. Amet nisl purus in mollis. " +
                                "Purus sit amet volutpat consequat mauris nunc congue nisi vitae. Pulvinar neque laoreet suspendisse interdum consectetur libero id faucibus. " +
                                "Leo integer malesuada nunc vel. Fringilla est ullamcorper eget nulla facilisi etiam dignissim diam quis. Augue interdum velit euismod in pellentesque. " +
                                "Tortor condimentum lacinia quis vel eros donec ac odio. Porttitor eget dolor morbi non arcu risus.\n\n";

            byte[] bytes = Encoding.UTF8.GetBytes(text);

            byte[] compressed = ccrush.Compress(bytes, 8);
            byte[] decompressed = ccrush.Decompress(compressed);

            Console.WriteLine(string.Format("Compressed length: {0} B", compressed.Length));
            Console.WriteLine(string.Format("Decompressed length: {0} B", decompressed.Length));

            ccrush.Dispose();
        }
    }
}