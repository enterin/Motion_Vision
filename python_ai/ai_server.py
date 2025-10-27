import socket
import struct
import json
import time
import cv2
import numpy as np
import torch
import base64
from ultralytics import YOLO

# ===========================================
# YOLO 설정
# ===========================================
DEVICE = 0 if torch.cuda.is_available() else "cpu"
YOLO_WEIGHTS = "yolov8n.pt"   # 학습 후 모델로 교체


_yolo_model = None

def get_yolo():
    global _yolo_model
    if _yolo_model is None:
        print(f"[YOLO] Loading model from {YOLO_WEIGHTS} on device {DEVICE} ...")
        _yolo_model = YOLO(YOLO_WEIGHTS)
    return _yolo_model

# ===========================================
# YOLO 추론
# ===========================================
def infer_image(img_bgr, camera_type=None):
    t0 = time.time()
    img_rgb = cv2.cvtColor(img_bgr, cv2.COLOR_BGR2RGB)
    model = get_yolo()
    results = model.predict(
        source=img_rgb,
        imgsz=640,
        conf=0.25,
        iou=0.45,
        device=DEVICE,
        verbose=False
    )
    r = results[0]
    boxes = r.boxes
    names = r.names
    bboxes = []
    max_conf = 0.0

    if boxes is not None and len(boxes) > 0:
        xyxy = boxes.xyxy.cpu().numpy()
        conf = boxes.conf.cpu().numpy()
        cls = boxes.cls.cpu().numpy().astype(int)
        for (x1, y1, x2, y2), c, k in zip(xyxy, conf, cls):
            label_name = names.get(k, str(k))
            w, h = int(x2 - x1), int(y2 - y1)
            bboxes.append({
                "x": int(x1),
                "y": int(y1),
                "w": w,
                "h": h,
                "label": label_name,
                "score": float(c)
            })
            if c > max_conf:
                max_conf = float(c)

    result = "normal"
    if len(bboxes) > 0:
        result = "defective"

    t1 = time.time()
    return {
        "ok": True,
        "camera_type": camera_type,
        "result": result,
        "defect_score": float(max_conf),
        "classes": [{"name": bb["label"], "score": bb["score"]} for bb in bboxes],
        "bboxes": bboxes,
        "inference_ms": (t1 - t0) * 1000.0,
        "model_version": f"yolo8-{YOLO_WEIGHTS}",
    }

# ===========================================
# TCP 서버
# ===========================================
HOST = '127.0.0.1'
PORT = 8008

def recv_all(sock, length):
    buf = b''
    while len(buf) < length:
        data = sock.recv(length - len(buf))
        if not data:
            return None
        buf += data
    return buf

def handle_client(conn, addr):
    print(f"[+] Connected by {addr}")
    try:
        while True:
            header = recv_all(conn, 5)
            if not header:
                print("[-] No header, closing.")
                break

            total_len, msg_type = struct.unpack('<IB', header)
            data_len = total_len - 5

            data = recv_all(conn, data_len)
            if not data:
                print("[-] No data, closing.")
                break

            if msg_type == 1:
                # ✅ JSON 파싱 후 Base64 디코드
                json_str = data.decode('utf-8')
                payload = json.loads(json_str)
                img_b64 = payload["image"].strip()
                camera_type = payload.get("camera_type", None)

                jpg_bytes = base64.b64decode(img_b64)
                img = cv2.imdecode(np.frombuffer(jpg_bytes, np.uint8), cv2.IMREAD_COLOR)
                result = infer_image(img, camera_type)

                json_bytes = json.dumps(result).encode('utf-8')
                resp = struct.pack('<IB', len(json_bytes)+5, 1) + json_bytes
                conn.sendall(resp)

            elif msg_type == 3:
                print("[-] Client requested close.")
                break

    except Exception as e:
        print(f"[!] Error: {e}")

    finally:
        conn.close()
        print(f"[-] Disconnected {addr}")


def main():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind((HOST, PORT))
    s.listen()
    print(f"[AI Server] Listening on {HOST}:{PORT}")
    while True:
        conn, addr = s.accept()
        handle_client(conn, addr)

if __name__ == "__main__":
    main()
