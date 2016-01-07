package com.nxsie.m2n;

import org.libsdl.app.SDLActivity;

import android.os.*;
import android.view.View;
import android.content.res.AssetManager;

public class MainActivity extends SDLActivity {
    private static AssetManager assetMgr;

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        assetMgr = getApplicationContext().getAssets();

        getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_STABLE
            | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
            | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
            | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION // hide nav bar
            | View.SYSTEM_UI_FLAG_FULLSCREEN // hide status bar
        );
    }

    public static boolean fsIsDirectory(String path)
    {
        try {
            assetMgr.list(path);
            return true;
        }
        catch (Exception e)
        {
            return false;
        }
    }
    
    public static String fsListAssetDirectory(String path)
    {
        try {
            String[] files = assetMgr.list(path);

            StringBuilder sb = new StringBuilder();
            boolean first = true;
            for (String item : files) {
                if (first) {
                    first = false;
                }
                else {
                    sb.append(";");
                }
                sb.append(item);
            }

            return sb.toString();
        }
        catch (Exception e) {
            return "";
        }
    }
}
