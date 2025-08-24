package org.shahcoincore.qt;

import android.os.Bundle;
import android.system.ErrnoException;
import android.system.Os;

import org.qtproject.qt5.android.bindings.QtActivity;

import java.io.File;

public class ShahcoinQtActivity extends QtActivity
{
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        final File shahcoinDir = new File(getFilesDir().getAbsolutePath() + "/.shahcoin");
        if (!shahcoinDir.exists()) {
            shahcoinDir.mkdir();
        }

        super.onCreate(savedInstanceState);
    }
}
