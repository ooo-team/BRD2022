import cv2 as cv
import numpy as np
from PIL import Image

def get_countours(img) :
    imgray = cv.cvtColor(img, cv.COLOR_BGR2GRAY)
    imgray = cv.blur(imgray, (3,3))
    _, thresh = cv.threshold(imgray, 127, 255, 0)
    contours, _ = cv.findContours(thresh, cv.RETR_TREE, cv.CHAIN_APPROX_SIMPLE)
    im = np.zeros(img.shape, np.uint8)
    cv.drawContours(im, contours, -1, (255,255,255), 3)
    return im

# define a video capture object
vid = cv.VideoCapture(0)

while(True):
	
	# Capture the video frame
	# by frame
    _, frame = vid.read()

    proc_img = get_countours(frame)

    # print(proc_img.shape, frame.shape)

    horizontal_glue = np.concatenate((frame, proc_img), axis=1)

	# Display the resulting frame
    cv.imshow('frame', horizontal_glue)
	
	# the 'q' button is set as the
	# quitting button you may use any
	# desired button of your choice
    if cv.waitKey(1) & 0xFF == ord('q'):
        break

# After the loop release the cap object
vid.release()
# Destroy all the windows
cv.destroyAllWindows()
