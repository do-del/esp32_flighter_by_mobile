package gd.hq.yolov5;

import android.util.Log;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;


public class UDPBuild {
    private static UDPBuild udpBuild;
    private static final String TAG = "UDPBuild";
    //    单个CPU线程池大小
    private static final int POOL_SIZE = 5;
    private static final int BUFFER_LENGTH = 1024;
    private byte[] receiveByte = new byte[BUFFER_LENGTH];

    private boolean isThreadRunning = false;

    private DatagramSocket client;
    private DatagramPacket receivePacket;

    private ExecutorService mThreadPool;
    private Thread clientThread;

    private OnUDPReceiveCallbackBlock udpReceiveCallback;

    //    构造函数私有化
    private UDPBuild() {
        super();
        int cpuNumbers = Runtime.getRuntime().availableProcessors();
//        根据CPU数目初始化线程池
        mThreadPool = Executors.newFixedThreadPool(cpuNumbers * POOL_SIZE);
    }

    //    提供一个全局的静态方法
    public static UDPBuild getUdpBuild() {
        if (udpBuild == null) {
            synchronized (UDPBuild.class) {
                if (udpBuild == null) {
                    udpBuild = new UDPBuild();
                }
            }
        }
        return udpBuild;
    }

    public void startUDPSocket() {
        if (client != null) return;
        try {
//            表明这个 Socket 在设置的端口上监听数据。
            client = new DatagramSocket(Constants.SOCKET_UDP_PORT);

            if (receivePacket == null) {
                receivePacket = new DatagramPacket(receiveByte, BUFFER_LENGTH);
            }
            startSocketThread();
        } catch (SocketException e) {
            e.printStackTrace();
        }
    }

    /**
     * 开启发送数据的线程
     **/
    private void startSocketThread() {
        clientThread = new Thread(new Runnable() {
            @Override
            public void run() {
                Log.d(TAG, "clientThread is running...");
                receiveMessage();
            }
        });
        isThreadRunning = true;
        clientThread.start();
    }

    /**
     * 处理接受到的消息
     **/
    private void receiveMessage() {
        while (isThreadRunning) {
            if (client != null) {
                try {
                    client.receive(receivePacket);
                } catch (IOException e) {
                    Log.e(TAG, "UDP数据包接收失败！线程停止");
                    stopUDPSocket();
                    e.printStackTrace();
                    return;
                }
            }

            if (receivePacket == null || receivePacket.getLength() == 0) {
                Log.e(TAG, "无法接收UDP数据或者接收到的UDP数据为空");
                continue;
            }
            String strReceive = new String(receivePacket.getData(), 0, receivePacket.getLength());
            Log.d(TAG, strReceive + " from " + receivePacket.getAddress().getHostAddress() + ":" + receivePacket.getPort());
//            解析接收到的 json 信息
            if (udpReceiveCallback != null) {
                udpReceiveCallback.OnParserComplete(receivePacket);
            }
//            每次接收完UDP数据后，重置长度。否则可能会导致下次收到数据包被截断。
            if (receivePacket != null) {
                receivePacket.setLength(BUFFER_LENGTH);
            }
        }
    }

    /**
     * 停止UDP
     **/
    public void stopUDPSocket() {
        isThreadRunning = false;
        receivePacket = null;
        if (clientThread != null) {
            clientThread.interrupt();
        }
        if (client != null) {
            client.close();
            client = null;
        }
        removeCallback();
    }

    /**
     * 发送信息
     **/
    public void sendMessage(final String message) {
        if (client == null) {
            startUDPSocket();
        }
        mThreadPool.execute(new Runnable() {
            @Override
            public void run() {
                try {
                    InetAddress targetAddress = InetAddress.getByName(Constants.SOCKET_HOST);

                    DatagramPacket packet = new DatagramPacket(message.getBytes(), message.length(), targetAddress, Constants.SOCKET_UDP_PORT);
                    client.send(packet);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        });
    }

    public interface OnUDPReceiveCallbackBlock {
        void OnParserComplete(DatagramPacket data);
    }

    public void setUdpReceiveCallback(OnUDPReceiveCallbackBlock callback) {
        this.udpReceiveCallback = callback;
    }

    public void removeCallback() {
        udpReceiveCallback = null;
    }
}