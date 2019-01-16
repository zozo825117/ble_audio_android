/*
 * The application needs to have the permission to write to external storage
 * if the output file is written to the external storage, and also the
 * permission to record audio. These permissions must be set in the
 * application's AndroidManifest.xml file, with something like:
 *
 * <uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" />
 * <uses-permission android:name="android.permission.RECORD_AUDIO" />
 *
 */
package com.android.audiorecordtest;

import android.app.Activity;
import android.widget.CheckBox;
import android.widget.LinearLayout;
import android.widget.Toast;
import android.os.Bundle;
import android.os.Environment;
import android.view.ViewGroup;
import android.widget.Button;
import android.view.View;
import android.view.View.OnClickListener;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.speech.RecognizerIntent;
import android.util.Log;
import android.media.AudioManager;
import android.media.MediaRecorder;
import android.media.MediaPlayer;

import java.io.IOException;
import java.util.ArrayList;


public class AudioRecordTest extends Activity
{

    private static final String LOG_TAG = "AudioRecordTest";
    private static String mFileName = null;

    //private static final int  MODE_FORCE_BLE = 0xee;
    //private static final int  MODE_CLEAR_FORCE_BLE = 0xef;
    private RecordButton mRecordButton = null;
    private MediaRecorder mRecorder = null;
   

    private PlayButton   mPlayButton = null;
    private MediaPlayer   mPlayer = null;
    
    private SetForceUseButton  mSetForceButton = null;
    private CheckBox mGoogleVoiceChkBox = null;
    private boolean  mGoogleVoiceShowing = false;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        LinearLayout ll = new LinearLayout(this);
        mRecordButton = new RecordButton(this);
        ll.addView(mRecordButton,
            new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT,
                0));
        mPlayButton = new PlayButton(this);
        ll.addView(mPlayButton,
            new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT,
                0));
       
        mSetForceButton = new SetForceUseButton(this);
        mSetForceButton.setText("SetForceUse");
        ll.addView(mSetForceButton,
        	    new LinearLayout.LayoutParams(
                        ViewGroup.LayoutParams.WRAP_CONTENT,
                        ViewGroup.LayoutParams.WRAP_CONTENT,
                        0));
        
        
        mGoogleVoiceChkBox = new CheckBox(this);
        mGoogleVoiceChkBox.setText("Enable GoogleVoice");
        ll.addView(mGoogleVoiceChkBox,
        	    new LinearLayout.LayoutParams(
                        ViewGroup.LayoutParams.WRAP_CONTENT,
                        ViewGroup.LayoutParams.WRAP_CONTENT,
                        0));
        
        
        setContentView(ll);
        
        BroadcastReceiver receiver = new BroadcastReceiver() {
			@Override
			public void onReceive(Context context, Intent intent) {
				if (intent.getAction().equals("com.hzdusun.bleaudio.action.stagechange")) {
					int state = intent.getIntExtra("com.hzdusun.bleaudio.extra.newstate", 0);
					if (state == 1 && mGoogleVoiceChkBox.isChecked() && !mGoogleVoiceShowing ) {
						mGoogleVoiceShowing = true;
						SpeechRecognitionHelper.run(AudioRecordTest.this);
					}
				}
			}
        };
        
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction("com.hzdusun.bleaudio.action.stagechange"); 
        registerReceiver(receiver, intentFilter);
    }

    
    
    private void onRecord(boolean start) {
        if (start) {
            startRecording();
            
        } else {
            stopRecording();
        }
    }

    private void onPlay(boolean start) {
        if (start) {
            startPlaying();
        } else {
            stopPlaying();
        }
    }

    private void startPlaying() {
        mPlayer = new MediaPlayer();
        try {
            mPlayer.setDataSource(mFileName);
            mPlayer.prepare();
            mPlayer.start();
        } catch (IOException e) {
            Log.e(LOG_TAG, "prepare() failed");
        }
    }

    private void stopPlaying() {
        mPlayer.release();
        mPlayer = null;
    }

    private void startRecording() {
    	
    	AudioManager mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE); 
    	//mAudioManager.setMode(MODE_FORCE_BLE); 
    	 
        mRecorder = new MediaRecorder();
        mRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
        mRecorder.setOutputFormat(MediaRecorder.OutputFormat.THREE_GPP);
        mRecorder.setOutputFile(mFileName);
        Log.e(LOG_TAG, "Filename: " + mFileName);
        mRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.AAC);
        //mRecorder.setAudioSamplingRate(48000);
        mRecorder.setAudioSamplingRate(16000); 
       
        mRecorder.setAudioChannels(1);
        
        try {
           mRecorder.prepare();
        } catch (IOException e) {
            Log.e(LOG_TAG, "prepare() failed");
            Log.e(LOG_TAG, e.getMessage());
        }

        mRecorder.start();
    }

    private void stopRecording() {
        mRecorder.stop();
        mRecorder.release();
        mRecorder = null;
    }

    class RecordButton extends Button {
        boolean mStartRecording = true;

        OnClickListener clicker = new OnClickListener() {
            public void onClick(View v) {
                onRecord(mStartRecording);
                if (mStartRecording) {
                    setText("Stop recording");
                } else {
                    setText("Start recording");
                }
                mStartRecording = !mStartRecording;
            }
        };

        public RecordButton(Context ctx) {
            super(ctx);
            setText("Start recording");
            setOnClickListener(clicker);
        }
    }

    class PlayButton extends Button {
        boolean mStartPlaying = true;

        OnClickListener clicker = new OnClickListener() {
            public void onClick(View v) {
                onPlay(mStartPlaying);
                if (mStartPlaying) {
                    setText("Stop playing");
                } else {
                    setText("Start playing");
                }
                mStartPlaying = !mStartPlaying;
            }
        };

        public PlayButton(Context ctx) {
            super(ctx);
            setText("Start playing");
            setOnClickListener(clicker);
        }
    }
    
    class SetForceUseButton extends Button {
    	
    	 boolean setForced = false;
    	 AudioManager mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE); 
    	 
    	 OnClickListener clicker = new OnClickListener() {
             public void onClick(View v) {
                 setForced = !setForced; 
                 if (setForced) {
                     setText("unset SetForceUse");
                 } else {
                     setText("SetForceUse");
                 }
                 
               /*  if (setForced) {
                	 mAudioManager.setMode(MODE_FORCE_BLE); 
                 } else {
                	 mAudioManager.setMode(MODE_CLEAR_FORCE_BLE);
                 }*/
             }
         };
         
         public SetForceUseButton(Context ctx) {
             super(ctx);
             setText("SetForceUse");
             setOnClickListener(clicker);
         }
         
         
    }

    class GoogleVoiceButton extends Button {
        //boolean mStartRecording = true;

        OnClickListener clicker = new OnClickListener() {
            public void onClick(View v) {
                //onRecord(mStartRecording);
               /* if (mStartRecording) {
                    setText("Stop recording");
                } else {
                    setText("Start recording");
                }
                mStartRecording = !mStartRecording;*/
            	SpeechRecognitionHelper.run(AudioRecordTest.this);
            }
        };

        public GoogleVoiceButton(Context ctx) {
            super(ctx);
            setText("Start recording");
            setOnClickListener(clicker);
        }
    }

    
    
    public AudioRecordTest() {
        //mFileName = Environment.getExternalStorageDirectory().getAbsolutePath();
    	//TCL do not have sdcard dir
        mFileName = "/data/audio_ble_test/test.3gp";
    }

  
    
    @Override
   
        public void onActivityResult(int requestCode, int resultCode, Intent data) {
            // if it’s speech recognition results
           // and process finished ok
            if (requestCode == 911 && resultCode == RESULT_OK) {
                // receiving a result in string array
                // there can be some strings because sometimes speech recognizing inaccurate
                // more relevant results in the beginning of the list
           ArrayList matches = data.getStringArrayListExtra(RecognizerIntent.EXTRA_RESULTS);
                // in “matches” array we holding a results... let’s show the most relevant
                if (matches.size() > 0) Toast.makeText(this, (CharSequence) matches.get(0), Toast.LENGTH_LONG).show();
            }
            
            
            mGoogleVoiceShowing = false;
            super.onActivityResult(requestCode, resultCode, data);
        }

    
    @Override
    public void onPause() {
        super.onPause();
        if (mRecorder != null) {
            mRecorder.release();
            mRecorder = null;
        }

        if (mPlayer != null) {
            mPlayer.release();
            mPlayer = null;
        }
    }
}