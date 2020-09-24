﻿using System;
using System.IO;
using System.Linq;
using System.Text;
using System.Security.Cryptography;
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

        private CompressDelegate compressDelegate;
        private DecompressDelegate decompressDelegate;

        #endregion

        private IntPtr lib;
        private ISharedLibLoadUtils loadUtils = null;

        /// <summary>
        /// Absolute path to the ccrush shared library that is currently loaded into memory for CcrushSharp.
        /// </summary>
        public string LoadedLibraryPath { get; }

        /// <summary>
        /// Creates a new CcrushSharp instance. <para> </para>
        /// Make sure to create one only once and cache it as needed, since loading the DLLs into memory can negatively affect the performance.
        /// </summary>
        public CcrushSharpContext()
        {
            StringBuilder pathBuilder = new StringBuilder(256);
            pathBuilder.Append("lib/");

            switch (RuntimeInformation.ProcessArchitecture)
            {
                case Architecture.X64:
                    pathBuilder.Append("x64/");
                    break;
                case Architecture.X86:
                    pathBuilder.Append("x86/");
                    break;
                case Architecture.Arm:
                    pathBuilder.Append("armeabi-v7a/");
                    break;
                case Architecture.Arm64:
                    pathBuilder.Append("arm64-v8a/");
                    break;
            }

            if (!Directory.Exists(pathBuilder.ToString()))
            {
                throw new PlatformNotSupportedException($"Ccrush shared library not found in {pathBuilder.ToString()} and/or unsupported CPU architecture. Please don't forget to copy the ccrush shared libraries/DLL into the 'lib/{{CPU_ARCHITECTURE}}/{{OS}}/{{SHARED_LIB_FILE}}' folder of your output build directory.  https://github.com/GlitchedPolygons/ccrush/tree/master/csharp");
            }

            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                loadUtils = new SharedLibLoadUtilsWindows();
                pathBuilder.Append("windows/");
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
            {
                loadUtils = new SharedLibLoadUtilsLinux();
                pathBuilder.Append("linux/");
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                loadUtils = new SharedLibLoadUtilsMac();
                pathBuilder.Append("mac/");
            }
            else
            {
                throw new PlatformNotSupportedException("Unsupported OS");
            }

            string[] l = Directory.GetFiles(pathBuilder.ToString());
            if (l == null || l.Length != 1)
            {
                throw new FileLoadException("There should only be exactly one ccrush shared library file per supported platform!");
            }

            pathBuilder.Append(Path.GetFileName(l[0]));

            LoadedLibraryPath = Path.GetFullPath(pathBuilder.ToString());

            pathBuilder.Clear();

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

            compressDelegate = Marshal.GetDelegateForFunctionPointer<CompressDelegate>(compress);
            decompressDelegate = Marshal.GetDelegateForFunctionPointer<DecompressDelegate>(decompress);

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
            int r = compressDelegate(data, (ulong)data.LongLength, bufferSizeKiB, level, out IntPtr output, out ulong outputLength);
            switch (r)
            {
                case 0:
            }

            return r != 0 ? null : MarshalReadBytes(output, outputLength, bufferSizeKiB);
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
        }
    }
}