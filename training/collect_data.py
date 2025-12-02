import cv2
import os

def collect_from_video(video_path, output_dir):
    cap = cv2.VideoCapture(video_path)
    os.makedirs(f"{output_dir}/images", exist_ok=True)
    frame_id = 0
    while cap.isOpened():
        ret, frame = cap.read()
        if not ret: break
        cv2.imwrite(f"{output_dir}/images/frame_{frame_id}.jpg", frame)
        frame_id += 1
    cap.release()

# Usage: collect_from_video('gameplay.mp4', './dataset')
if __name__ == "__main__":
    collect_from_video('gameplay.mp4', './dataset')
