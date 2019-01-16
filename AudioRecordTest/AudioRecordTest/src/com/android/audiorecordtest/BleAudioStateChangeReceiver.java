package com.android.audiorecordtest;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.widget.Toast;

public class BleAudioStateChangeReceiver extends BroadcastReceiver {

	private static final String TAG = "BleAudioStateChangeReceiver";
	@Override
	public void onReceive(Context context, Intent intent) {
		int state = intent.getIntExtra("com.hzdusun.bleaudio.extra.newstate", 0);
		if (state == 0) {
			Log.e(TAG, "BLE DEVICE DISCONNECTED");
			Toast.makeText(context, "BLE DEVICE DISCONNECTED", Toast.LENGTH_SHORT).show();
		}else {
			Log.e(TAG, "BLE DEVICE CONNECTED");
			Toast.makeText(context, "BLE DEVICE CONNECTED", Toast.LENGTH_SHORT).show();
		}
	}
}
