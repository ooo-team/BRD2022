import cv2 as cv
import numpy as np
from PIL import Image


def get_countours(img):
    imgray = cv.Canny(img, 100, 200)
    _, thresh = cv.threshold(imgray, 127, 255, 0)
    contours, _ = cv.findContours(thresh, cv.RETR_TREE, cv.CHAIN_APPROX_SIMPLE)
    im = np.zeros(img.shape, np.uint8)
    cv.drawContours(im, contours, -1, (0, 255, 0), 3)
    return im, contours

def get_BB(img, contours):
    contours_poly = [None]*len(contours)
    boundRect = [None]*len(contours)
    centers = [None]*len(contours)
    radius = [None]*len(contours)
    for i, c in enumerate(contours):
        contours_poly[i] = cv.approxPolyDP(c, 3, True)
        boundRect[i] = cv.boundingRect(contours_poly[i])
        # centers[i], radius[i] = cv.minEnclosingCircle(contours_poly[i])
    im = np.zeros(img.shape, np.uint8)
    for i in range(len(contours)):
        color = (0, 255, 0)
        cv.drawContours(im, contours_poly, i, color)
        cv.rectangle(im, (int(boundRect[i][0]), int(boundRect[i][1])), \
          (int(boundRect[i][0]+boundRect[i][2]), int(boundRect[i][1]+boundRect[i][3])), color, 2)
        # cv.circle(im, (int(centers[i][0]), int(centers[i][1])), int(radius[i]), color, 2)
    return im

# define a video capture object
vid = cv.VideoCapture(0)

while True:

    # Capture the video frame
    # by frame
    _, frame = vid.read()

    proc_img, ctrs = get_countours(frame)
    bb_image = get_BB(frame, ctrs)

    horizontal_glue = np.concatenate((frame, proc_img, bb_image), axis=1)

    # Display the resulting frame
    cv.imshow("frame", horizontal_glue)

    # the 'q' button is set as the
    # quitting button you may use any
    # desired button of your choice
    if cv.waitKey(1) & 0xFF == ord("q"):
        break

# After the loop release the cap object
vid.release()
# Destroy all the windows
cv.destroyAllWindows()
