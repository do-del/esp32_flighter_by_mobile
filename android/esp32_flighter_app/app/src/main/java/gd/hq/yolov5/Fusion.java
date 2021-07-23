package gd.hq.yolov5;

import android.Manifest;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.media.ExifInterface;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;

import com.zerokol.views.joystickView.JoystickView;
import com.zerokol.views.joystickView.JoystickView.OnJoystickMoveListener;
import android.app.Activity;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.provider.MediaStore;
import android.view.View;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.lang.Math;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.Locale;


public class Fusion extends Activity implements View.OnClickListener{
    //    private TextView angleTextView;
//    private TextView powerTextView;
//    private TextView directionTextView;
    private static final String TAG = "MainActivity::";

    //yolo5参数
    private static final int REQUEST_EXTERNAL_STORAGE = 1;
    private static final int REQUEST_PICK_IMAGE = 2;
    private static String[] PERMISSIONS_STORAGE = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    };
    private ImageView resultImageView;
    private SeekBar nmsSeekBar;
    private SeekBar thresholdSeekBar;
    private TextView thresholdTextview;
    private double threshold = 0.3,nms_threshold = 0.7;
    //yolo5参数

    private TextView throttletextview;
    private TextView yawtextview;
    private TextView pitchtextview;
    private TextView rolltextview;
    private int throttle;
    private int yaw;
    private int roll;
    private int pitch;

    // Importing also other views
    private JoystickView joystick;
    private JoystickView joystick1;

    private UDPBuild udpBuild;

    private final int op_camera = 1;
    private ImageView imageView;
    private HandlerThread handlerThread;
    private Handler handler;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.joystick_test);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

        int permission = ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE);
        if (permission != PackageManager.PERMISSION_GRANTED) {
            // We don't have permission so prompt the user
            ActivityCompat.requestPermissions(
                    this,
                    PERMISSIONS_STORAGE,
                    REQUEST_EXTERNAL_STORAGE
            );
        }

        YOLOv5.init(getAssets());
        thresholdTextview = findViewById(R.id.valTxtView);
        nmsSeekBar = findViewById(R.id.nms_seek);
        thresholdSeekBar = findViewById(R.id.threshold_seek);
        thresholdTextview.setText(String.format(Locale.ENGLISH,"Thresh:%.2f,NMS:%.2f",threshold,nms_threshold));
        nmsSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                nms_threshold = i/100.f;
                thresholdTextview.setText(String.format(Locale.ENGLISH,"Thresh:%.2f,NMS:%.2f",threshold,nms_threshold));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
        thresholdSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                threshold = i/100.f;
                thresholdTextview.setText(String.format(Locale.ENGLISH,"Thresh:%.2f,NMS:%.2f",threshold,nms_threshold));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        findViewById(R.id.op_cam).setOnClickListener(this);
        imageView = findViewById(R.id.img);

        handlerThread = new HandlerThread("http");
        handlerThread.start();
        handler = new HttpHandler(handlerThread.getLooper());

        udpBuild = UDPBuild.getUdpBuild();

//        angleTextView = (TextView) findViewById(R.id.angleTextView);
//        powerTextView = (TextView) findViewById(R.id.powerTextView);
//        directionTextView = (TextView) findViewById(R.id.directionTextView);

        throttletextview = (TextView) findViewById(R.id.channel3Textvalue);
        yawtextview = (TextView) findViewById(R.id.channel4Textvalue);
        pitchtextview = (TextView) findViewById(R.id.channel2Textvalue);
        rolltextview = (TextView) findViewById(R.id.channel1Textvalue);


        //Referencing also other views
        joystick = (JoystickView) findViewById(R.id.joystickView);
        joystick1 = (JoystickView) findViewById(R.id.joystickView1);

        //Event listener that always returns the variation of the angle in degrees, motion power in percentage and direction of movement
        joystick.setOnJoystickMoveListener(new OnJoystickMoveListener() {

            @Override
            public void onValueChanged(int angle, int power, int direction) {

                throttle = (int) (1500+power*Math.cos(Math.toRadians(angle))*5*Math.sqrt(2));
                if(throttle >= 2000){
                    throttle = 2000;
                }
                if(throttle <= 1000){
                    throttle = 1000;
                }

                yaw = (int) (1500+power*Math.sin(Math.toRadians(angle))*5*Math.sqrt(2));
                if(yaw >= 2000){
                    yaw = 2000;
                }
                if(yaw <= 1000){
                    yaw = 1000;
                }

                throttletextview.setText(String.valueOf(throttle));
                yawtextview.setText(String.valueOf(yaw));

                String message = String.valueOf(roll)+","+String.valueOf(pitch)+","+String.valueOf(throttle)+","+String.valueOf(yaw);
                udpBuild.sendMessage(message);
            }
        }, JoystickView.DEFAULT_LOOP_INTERVAL);

        joystick1.setOnJoystickMoveListener(new OnJoystickMoveListener() {

            @Override
            public void onValueChanged(int angle, int power, int direction) {

                roll = (int) (1500+power*Math.sin(Math.toRadians(angle))*5*Math.sqrt(2));
                if(roll >= 2000){
                    roll = 2000;
                }
                if(roll <= 1000){
                    roll = 1000;
                }

                pitch = (int) (1500-power*Math.cos(Math.toRadians(angle))*5*Math.sqrt(2));
                if(pitch >= 2000){
                    pitch = 2000;
                }
                if(pitch <= 1000){
                    pitch = 1000;
                }

                pitchtextview.setText(String.valueOf(pitch));
                rolltextview.setText(String.valueOf(roll));

                String message = String.valueOf(roll)+","+String.valueOf(pitch)+","+String.valueOf(throttle)+","+String.valueOf(yaw);
                udpBuild.sendMessage(message);
            }
        }, JoystickView.DEFAULT_LOOP_INTERVAL);
    }

    @Override
    public void onClick(View v)
    {
        switch (v.getId())
        {
            case R.id.op_cam:
                handler.sendEmptyMessage(op_camera);
                break;
            default:
                break;
        }
    }

    //动态申请权限
    public static boolean isGrantExternalRW(Activity activity) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M && activity.checkSelfPermission(
                Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {

            activity.requestPermissions(new String[]{
                    Manifest.permission.READ_EXTERNAL_STORAGE,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE
            }, 1);
            return false;
        }
        return true;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        for(int result:grantResults){
            if(result != PackageManager.PERMISSION_GRANTED){
                this.finish();
            }
        }
    }

    public Bitmap getPicture(String picturePath) {
//        String[] filePathColumn = { MediaStore.Images.Media.DATA };
//        Cursor cursor = this.getContentResolver().query(selectedImage, filePathColumn, null, null, null);
//        cursor.moveToFirst();
//        int columnIndex = cursor.getColumnIndex(filePathColumn[0]);
//        String picturePath = cursor.getString(columnIndex);
//        cursor.close();
        Bitmap bitmap = BitmapFactory.decodeFile(picturePath);
        //int rotate = readPictureDegree(picturePath);

        return rotateBitmapByDegree(bitmap,0);
    }

    public int readPictureDegree(String path) {
        int degree = 0;
        try {
            ExifInterface exifInterface = new ExifInterface(path);
            int orientation = exifInterface.getAttributeInt(ExifInterface.TAG_ORIENTATION, ExifInterface.ORIENTATION_NORMAL);
            switch (orientation) {
                case ExifInterface.ORIENTATION_ROTATE_90:
                    degree = 90;
                    break;
                case ExifInterface.ORIENTATION_ROTATE_180:
                    degree = 180;
                    break;
                case ExifInterface.ORIENTATION_ROTATE_270:
                    degree = 270;
                    break;
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        return degree;
    }

    public void pitcture_yolo(Bitmap image){
        //Bitmap image = getPicture(bitmap);
        Box[] result = YOLOv5.detect(image,threshold,nms_threshold);
        Bitmap mutableBitmap = image.copy(Bitmap.Config.ARGB_8888, true);
        Canvas canvas = new Canvas(mutableBitmap);
        final Paint boxPaint = new Paint();
        boxPaint.setAlpha(200);
        boxPaint.setStyle(Paint.Style.STROKE);
        boxPaint.setStrokeWidth(4 * image.getWidth()/800);
        boxPaint.setTextSize(40 * image.getWidth()/800);
        for(Box box:result){
            boxPaint.setColor(box.getColor());
            boxPaint.setStyle(Paint.Style.FILL);
            canvas.drawText(box.getLabel(),box.x0,box.y0,boxPaint);
            boxPaint.setStyle(Paint.Style.STROKE);
            canvas.drawRect(box.getRect(),boxPaint);
        }
        imageView.setImageBitmap(mutableBitmap);
    }

    public Bitmap rotateBitmapByDegree(Bitmap bm, int degree) {
        Bitmap returnBm = null;
        Matrix matrix = new Matrix();
        matrix.postRotate(degree);
        try {
            returnBm = Bitmap.createBitmap(bm, 0, 0, bm.getWidth(),
                    bm.getHeight(), matrix, true);
        } catch (OutOfMemoryError e) {
        }
        if (returnBm == null) {
            returnBm = bm;
        }
        if (bm != returnBm) {
            bm.recycle();
        }
        return returnBm;
    }

    private class HttpHandler extends Handler
    {
        public HttpHandler(Looper looper)
        {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg)
        {
            switch (msg.what)
            {
                case op_camera:
                    downloadFile();
                    break;
                default:
                    break;
            }
        }
    }

    private void downloadFile()
    {
        String downloadUrl = "http://192.168.4.1";
        String savePath = Environment.getExternalStorageDirectory().getAbsolutePath() + "/" + "pic.jpg";

        File file = new File(savePath);

        if (file.exists())
        {
            file.delete();
        }

//        if(!isGrantExternalRW(this)){
//            return;
//        }

        BufferedInputStream bufferedInputStream = null;
        FileOutputStream outputStream = null;
        try
        {
            URL url = new URL(downloadUrl);

            try
            {
                HttpURLConnection httpURLConnection = (HttpURLConnection) url.openConnection();
                httpURLConnection.setRequestMethod("GET");
                httpURLConnection.setConnectTimeout(1000 * 5);
                httpURLConnection.setReadTimeout(1000 * 5);
                httpURLConnection.setDoInput(true);
                httpURLConnection.connect();

                if (httpURLConnection.getResponseCode() == 200)
                {
                    InputStream in = httpURLConnection.getInputStream();

                    InputStreamReader isr = new InputStreamReader(in);
                    BufferedReader bufferedReader = new BufferedReader(isr);

                    String line;
                    StringBuffer stringBuffer = new StringBuffer();

                    int i = 0;

                    int len;
                    byte[] buffer;

                    while ((line = bufferedReader.readLine()) != null)
                    {
                        if (line.contains("Content-Type:"))
                        {
                            line = bufferedReader.readLine();

                            len = Integer.parseInt(line.split(":")[1].trim());

                            bufferedInputStream = new BufferedInputStream(in);
                            buffer = new byte[len];

                            int t = 0;
                            while (t < len)
                            {
                                t += bufferedInputStream.read(buffer, t, len - t);
                            }

                            bytesToImageFile(buffer,file);

                            final Bitmap bitmap = getPicture(savePath);
                            //final Bitmap bitmap = BitmapFactory.decodeByteArray(buffer,0,len);
                            runOnUiThread(new Runnable()
                            {
                                @Override
                                public void run()
                                {
                                    //imageView.setImageBitmap(bitmap);
                                    pitcture_yolo(bitmap);
                                }
                            });
                        }


                    }
                }

            } catch (IOException e)
            {
                e.printStackTrace();
            }
        } catch (MalformedURLException e)
        {
            e.printStackTrace();
        } finally
        {
            try
            {
                if (bufferedInputStream != null)
                {
                    bufferedInputStream.close();
                }
                if (outputStream != null)
                {
                    outputStream.close();
                }
            } catch (IOException e)
            {
                e.printStackTrace();
            }
        }

    }

    public Bitmap Bytes2Bimap(byte[] b) {
        if (b.length != 0) {
            return BitmapFactory.decodeByteArray(b, 0, b.length);
        } else {
            return null;
        }
    }

    private void bytesToImageFile(byte[] bytes, File file)
    {
        try
        {
            FileOutputStream fos = new FileOutputStream(file);
            fos.write(bytes, 0, bytes.length);
            fos.flush();
            fos.close();
        } catch (Exception e)
        {
            e.printStackTrace();
        }
    }
}
