package com.android.dreams.phototable;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.widget.Toast;

/**
 * Created by waha on 2018/1/16.
 */

public class CheckPermissionActivity extends Activity {
    private final int REQUEST_CODE_ASK_PERMISSIONS = 120;
    private static final String[] REQUEST_PERMISSIONS = new String[]{
            Manifest.permission.READ_EXTERNAL_STORAGE
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        requestPermissions(REQUEST_PERMISSIONS, REQUEST_CODE_ASK_PERMISSIONS);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        switch (requestCode) {
            case REQUEST_CODE_ASK_PERMISSIONS:
                for (int result : grantResults) {
                    if (result != PackageManager.PERMISSION_GRANTED) {
                        // Permission Denied
                        String toast_text = getResources().getString(R.string.err_permission);
                        Toast.makeText(CheckPermissionActivity.this, toast_text,
                                Toast.LENGTH_SHORT).show();
                        finish();
                        return;
                    }
                }
                // Permission Granted
                back2Activity();
                break;
            default:
                super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        }

    }

    private void back2Activity() {
        boolean isFromTable = getIntent().getBooleanExtra("isTable", false);
        Intent intent = new Intent(this, isFromTable ?
                PhotoTableDreamSettings.class : FlipperDreamSettings.class);
        startActivity(intent);
        finish();
    }

    public static boolean jump2PermissionActivity(Activity activity, boolean isTable) {
        for (String permission : REQUEST_PERMISSIONS) {
            if (PackageManager.PERMISSION_GRANTED != activity.checkSelfPermission(permission)) {
                Intent intent = new Intent(activity, CheckPermissionActivity.class);
                intent.putExtra("isTable", isTable);
                activity.startActivity(intent);
                return true;
            }
        }
        return false;
    }
}
