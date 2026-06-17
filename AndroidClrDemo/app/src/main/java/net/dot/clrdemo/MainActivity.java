// Minimal MainActivity — loads libclrdemo.so, extracts assets.zip,
// initializes CoreCLR, and calls ManagedClass.PrintMessage.
//
// This is a stripped-down version of the official Android sample.
// No Instrumentation, no test harness — just the CoreCLR essentials.

package net.dot.clrdemo;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.widget.TextView;
import android.graphics.Color;
import android.content.res.AssetManager;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.BufferedInputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

public class MainActivity extends Activity
{
    static { System.loadLibrary("clrdemo"); }

    // ── native methods (implemented in jni/clrdemo.c) ──────────

    static native int  initRuntime(String filesDir, String entryPointLibName);
    static native int  execEntryPoint(String entryPointLibName);
    static native void setEnv(String key, String value);
    static native void freeNativeResources();

    // ── asset extraction ───────────────────────────────────────

    static void unzipAssets(AssetManager am, String toDir, String zipName)
    {
        try {
            InputStream is = am.open(zipName);
            ZipInputStream zis = new ZipInputStream(new BufferedInputStream(is));
            byte[] buf = new byte[4096];
            ZipEntry ze;
            while ((ze = zis.getNextEntry()) != null)
            {
                File f = new File(toDir, ze.getName());
                if (ze.isDirectory()) { f.mkdirs(); continue; }
                f.getParentFile().mkdirs();

                FileOutputStream fos = new FileOutputStream(f);
                int n;
                while ((n = zis.read(buf)) != -1) fos.write(buf, 0, n);
                fos.close();
                zis.closeEntry();
                Log.i("CLRDEMO", "Extracted: " + f.getAbsolutePath());
            }
            zis.close();
        } catch (IOException e) {
            Log.e("CLRDEMO", "unzipAssets: " + e.getMessage());
        }
    }

    // ── lifecycle ──────────────────────────────────────────────

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        final TextView tv = new TextView(this);
        tv.setTextSize(18);
        tv.setTextColor(Color.WHITE);
        setContentView(tv);

        final String entryPoint = "ManagedDemo.dll";
        final String filesDir   = getFilesDir().getAbsolutePath();
        final String cacheDir   = getCacheDir().getAbsolutePath();

        tv.setText("Extracting assets...");

        // 1. Unzip managed DLLs + BCL from assets.zip
        unzipAssets(getAssets(), filesDir, "assets.zip");

        // 2. Set env for managed code
        setEnv("HOME", filesDir);
        setEnv("TMPDIR", cacheDir);

        // 3. Init runtime on background thread
        new Thread(new Runnable() {
            public void run() {
                int rc = initRuntime(filesDir, entryPoint);
                Log.i("CLRDEMO", "initRuntime => " + rc);

                if (rc != 0) {
                    final int code = rc;
                    runOnUiThread(new Runnable() {
                        public void run() {
                            tv.setText("CoreCLR init FAILED: 0x" + Integer.toHexString(code));
                        }
                    });
                    return;
                }

                // 4. Delay 1s then execute managed entry point
                new Handler(Looper.getMainLooper()).postDelayed(new Runnable() {
                    public void run() {
                        int ret = execEntryPoint(entryPoint);
                        Log.i("CLRDEMO", "execEntryPoint => " + ret);
                        tv.setText("CoreCLR returned: " + ret);
                    }
                }, 1000);
            }
        }).start();
    }

    @Override
    protected void onDestroy()
    {
        Log.i("CLRDEMO", "onDestroy");
        freeNativeResources();
        super.onDestroy();
    }

}
