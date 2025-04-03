import cv2
import numpy as np
import math
import board
import busio
from adafruit_ssd1306 import SSD1306_I2C
from PIL import Image, ImageDraw, ImageFont

# --- OLED SETUP ---
i2c = busio.I2C(board.SCL, board.SDA)
display = SSD1306_I2C(128, 64, i2c)
font = ImageFont.load_default()

def show_on_oled(message):
    display.fill(0)
    image = Image.new("1", (display.width, display.height))
    draw = ImageDraw.Draw(image)
    draw.text((0, 0), message, font=font, fill=255)
    display.image(image)
    display.show()

# --- CAMERA SETUP ---
cap = cv2.VideoCapture(0)

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = cv2.flip(frame, 1)
    roi = frame[100:400, 100:400]
    cv2.rectangle(frame, (100, 100), (400, 400), (0, 255, 0), 2)

    hsv = cv2.cvtColor(roi, cv2.COLOR_BGR2HSV)
    lower_skin = np.array([0, 20, 70], dtype=np.uint8)
    upper_skin = np.array([20, 255, 255], dtype=np.uint8)

    mask = cv2.inRange(hsv, lower_skin, upper_skin)
    kernel = np.ones((3, 3), np.uint8)
    mask = cv2.dilate(mask, kernel, iterations=4)
    mask = cv2.GaussianBlur(mask, (5, 5), 100)

    contours, _ = cv2.findContours(mask, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

    fingers = 0

    if contours:
        contour = max(contours, key=lambda x: cv2.contourArea(x))
        hull = cv2.convexHull(contour, returnPoints=False)
        defects = cv2.convexityDefects(contour, hull)

        if defects is not None:
            count_defects = 0
            for i in range(defects.shape[0]):
                s, e, f, d = defects[i, 0]
                start = tuple(contour[s][0])
                end = tuple(contour[e][0])
                far = tuple(contour[f][0])

                a = math.dist(start, end)
                b = math.dist(start, far)
                c = math.dist(end, far)
                angle = math.acos((b**2 + c**2 - a**2) / (2*b*c)) * 57

                if angle <= 90:
                    count_defects += 1
                    cv2.circle(roi, far, 5, (0, 0, 255), -1)

            fingers = count_defects + 1
        else:
            fingers = 1

    # Show count on camera feed
    cv2.putText(frame, f"Fingers: {fingers}", (10, 40),
                cv2.FONT_HERSHEY_SIMPLEX, 1.2, (255, 0, 0), 2)

    # Show count on OLED
    show_on_oled(f"Fingers: {fingers}")

    cv2.imshow("Finger Counter", frame)

    if cv2.waitKey(1) & 0xFF == ord("q"):
        break

cap.release()
cv2.destroyAllWindows()
