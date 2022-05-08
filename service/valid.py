from typing import Iterable
import cv2 as cv
import numpy as np
from enum import Enum

x_thresh, y_thresh = 45, 45


class DirectionCodes(Enum):
    FORWARD = 0
    LEFT = 1
    RIGHT = 2
    BACK = 3
    STOP_ALL = 4


def get_countours(img: np.ndarray) -> np.ndarray:
    """
    Get contours from incoming image.

    :param img: 3-dimensional image array
    :return: array of contours in the image
    """
    imgray = cv.Canny(img, 100, 200)
    _, thresh = cv.threshold(imgray, 127, 255, 0)
    contours, _ = cv.findContours(thresh, cv.RETR_TREE, cv.CHAIN_APPROX_SIMPLE)
    return contours


def get_BB(contours: np.ndarray) -> Iterable:
    """
    Get bounding boxes of the image.

    :param contours: array of contours
    :return: filter-object of bounding boxes that fit the size criteria
    """
    contours_poly = [None] * len(contours)
    boundRect = [None] * len(contours)
    for i, c in enumerate(contours):
        contours_poly[i] = cv.approxPolyDP(c, 3, True)
        boundRect[i] = cv.boundingRect(contours_poly[i])
    return filter(lambda box: box[2] > x_thresh and box[3] > y_thresh, boundRect)


def evaluate(img_shape: tuple[int, int], boxes: Iterable) -> DirectionCodes:
    """
    Choose which path to take.

    :param boxes: list of bounding boxes
    :return: code of movement
    """
    centers = map(lambda box: (box[0] + box[2] / 2, box[1] + box[3] / 2), boxes)
    areas = map(lambda box: box[2] * box[3], boxes)
    left_boxes = filter(lambda box: box[0][1] < img_shape[1] / 2, zip(centers, areas))
    right_boxes = filter(lambda box: box[0][1] > img_shape[1] / 2, zip(centers, areas))
    left_sum = sum([v[1] for v in left_boxes])
    right_sum = sum([v[1] for v in right_boxes])

    return DirectionCodes.LEFT if right_sum > left_sum else DirectionCodes.RIGHT


vid = cv.VideoCapture(2)

while True:
    _, frame = vid.read()

    ctrs = get_countours(frame)
    rects = get_BB(ctrs)
    

    direction_code = evaluate(rects)
