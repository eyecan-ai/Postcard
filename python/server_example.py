import random
import cv2
import numpy as np
import postcard


def imageCallback(header, input_image):
    print("New image received")
    print(header)
    print(input_image.shape)

    #Draw random circles
    center = np.random.randint(0,min(input_image.shape[0], input_image.shape[1]),(2,))
    cv2.circle(input_image, tuple(center), 15, (255,0,255),-1)
    
    return "ok", input_image


socket = postcard.PostcardServer.AcceptingSocket('0.0.0.0', 8000)
print("Server Running...")

while True:
    print("Waiting for connection....")
    server = postcard.PostcardServer.newServer(socket, data_callback=imageCallback)
