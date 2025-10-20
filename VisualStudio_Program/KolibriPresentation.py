import serial
import numpy as np
import matplotlib.pyplot as plt
import time
from matplotlib import cm
import cv2 as cv

def clamp(n, minn, maxn):
    if n < minn:
        return minn
    elif n > maxn:
        return maxn
    else:
        return n
    
def Sphere(R):

    
    Size1 = len(R) #8*8
    Size2 = len(R[0]) #8*5
    print(Size2, Size1)

    # phi = np.linspace(0, ((90  - 22.5)*np.pi)/180, Size2)
    # theta = np.linspace(0, 2*np.pi, Size1)
    # print(len(phi))


    z = np.zeros((Size1, Size2))
    x = np.zeros((Size1, Size2))
    y = np.zeros((Size1, Size2))

    phi = np.linspace(np.pi/8, 7*np.pi/8, Size2)
    theta = np.linspace(0, np.pi, Size1)

    for t in range(len(theta)):
        for p in range(len(phi)):
            x[t][p] = R[t][p]*np.cos(theta[t])*np.cos(phi[p])
            y[t][p] = R[t][p]*np.sin(theta[t])*np.cos(phi[p])
            z[t][p] = R[t][p]*np.sin(phi[p])

    # for p in range(len(phi)):
    #     for t in range(len(theta)): 
    #         x[t][p] = R[t][p]*np.cos(theta[t])*np.sin(phi[p])
    #         y[t][p] = R[t][p]*np.sin(theta[t])*np.sin(phi[p])
    #         z[t][p] = R[t][p]*np.cos(phi[p])

    return x, y, z

def AcquireOneLine_Converted(ImaqArrayData):
    Data = np.zeros(66)
    T_array = [38,83,100,83,38]

    for p in range(5):
        s = ser.read(132) #prebere podatke
        for i in range(66):
            Data[i] = clamp(((int.from_bytes([s[1 + 2*i], s[2*i]], byteorder='big', signed=False))*T_array[p])/100, 80, 200) #iz bajtov naredi 16bitne vrednosti in jih pretvori v cilindrične

        SingleImaqArray = Data[2:66] #izreze ven samo podatke
        SingleImaqArray = SingleImaqArray.reshape((8,8)) # naredi matriko 8x8
        print(s[2])
        if s[2] == 51:
            SingleImaqArray = np.rot90(SingleImaqArray, k=-1) #Zarotira array, ker je senzor v sredini drgace obrnjen

        ImaqArrayData = np.concatenate((ImaqArrayData, SingleImaqArray), axis=1) #Pripopa senzor celotni sliki

    
    ImaqArrayData = np.delete(ImaqArrayData, 0, axis=1)
    ImaqArrayData = np.uint16(ImaqArrayData) #pretvori vse skupaj v nepredznačena cela števila

    return ImaqArrayData, Data[0] #Vrne celotno vrstico meritve in pozicijo kje je bilo to zajeto

def AcquireOneLine_Raw(ImaqArrayData):
    Data = np.zeros(66)

    for p in range(5):
        s = ser.read(132) #prebere podatke
        for i in range(66):
            Data[i] = clamp((int.from_bytes([s[1 + 2*i], s[2*i]], byteorder='big', signed=False)), 30, 200) #iz bajtov naredi 16bitne vrednosti in jih pretvori v cilindrične

        SingleImaqArray = Data[2:66] #izreze ven samo podatke
        SingleImaqArray = SingleImaqArray.reshape((8,8)) # naredi matriko 8x8
        if s[2] == 51:
            SingleImaqArray = np.rot90(SingleImaqArray, k=-1) #Zarotira array, ker je senzor v sredini drgace obrnjen

        ImaqArrayData = np.concatenate((ImaqArrayData, SingleImaqArray), axis=1) #Pripopa senzor celotni sliki

    ImaqArrayData = np.delete(ImaqArrayData, 0, axis=1)
    ImaqArrayData = np.uint16(ImaqArrayData) #pretvori vse skupaj v nepredznačena cela števila

    return ImaqArrayData, Data[0] #Vrne celotno vrstico meritve in pozicijo kje je bilo to zajeto


ImaqArray = np.zeros((4,8,40))


ImageNum = 0
DvaD_TriD = 0
OneLine = 1
WholeSphere = 0

#fig = plt.figure()
#ax = plt.axes(projection ='3d')

fig = plt.figure(figsize=plt.figaspect(2.))
ax2 = fig.add_subplot(2, 1, 2, projection='3d')
ax1 = fig.add_subplot(2, 1, 1)

plt.ion()

print("Starting...")
ser = serial.Serial('COM3', 2000000)

while(1):


    tic = time.time()

    if ser.write(b'\x37') == 1:
        if WholeSphere == 1:
            R_Data_Array = np.zeros((1,40))
            
            for i in range(8):
                ImaqArrayData = np.zeros((8,1))
                ImaqArrayData, Position = AcquireOneLine_Raw(ImaqArrayData)
            
                R_Data_Array = np.concatenate((R_Data_Array, ImaqArrayData), axis=0)
                print("Position ", Position)
            
            R_Data_Array = np.delete(R_Data_Array, 0, axis=0)
            R_Data_Array = cv.resize(R_Data_Array,(320, 64*8), interpolation = cv.INTER_CUBIC)
            sharpen_filter=np.array([[0,-1,0],[-1,5,-1],[0,-1,0]])

            image= cv.filter2D(R_Data_Array,-1,sharpen_filter)
            image = cv.erode(image, (3,3))
            image = cv.dilate(image, (3,3))
            #image = cv.blur(image,(5,5)) #cv.GaussianBlur(image, (5,5), 1)
            image = cv.medianBlur(np.uint8(image), 27)

            x,y,z = Sphere(image)

            if DvaD_TriD == 1:
                fig = plt.figure()
                ax = plt.axes(projection ='3d')
                ax.cla()
                ax.plot_surface(x, y, z, cmap=cm.Blues) 
                plt.show(block = True)
                #plt.draw()
                plt.pause(0.001)
            else:
                plt.clf()
                plt.pcolormesh(z)
                #plt.imshow(image, interpolation="hanning")
                plt.draw()
                plt.pause(0.001)

        if OneLine == 1:

            X, Y = np.meshgrid(range(320), range(64))  # `plot_surface` expects `x` and `y` data to be 2D
            
            ImaqArrayData = np.zeros((8,1))
            ImaqArray[0], Position = AcquireOneLine_Converted(ImaqArrayData)
            

            image = cv.resize(ImaqArray[0],(320, 64), interpolation = cv.INTER_CUBIC)
            ret,image = cv.threshold(image,200,30,cv.THRESH_TRUNC)
            sharpen_filter=np.array([[0,-1,0],[-1,5,-1],[0,-1,0]])

            #image= cv.filter2D(image,-1,sharpen_filter)
            image = cv.blur(image,(3,3)) #cv.GaussianBlur(image, (5,5), 1)
            image = cv.medianBlur(np.uint8(image), 3)
            #Gradient = np.gradient(ImaqArray, 2) #ARR[[stolpec1, stolpec2]], ARR[[vrstica1, vrstica2]], ARR[[globina1], [globina2]]

            if DvaD_TriD == 1:

                ax.cla()
                ax.plot_surface(X, Y, image, cmap=cm.Blues) 
                plt.draw()
                plt.pause(0.001)
            else:

                # ax.cla()
                # ax.plot_surface(X, Y, image, cmap=cm.Blues)
                # plt.draw()

                # plt.clf()
                # plt.pcolormesh(image)
                # #plt.imshow(image, interpolation="hanning")
                # plt.draw()
                # plt.pause(0.001)
                ax1.cla()
                ax2.cla()

                ax1.pcolormesh(image)
                ax1.grid(True)

                # Second subplot
                ax2.plot_surface(X, Y, image, cmap=cm.Blues)

                plt.draw()
                plt.pause(0.01)
    else:
        time.sleep(1)



    toc = time.time()
    print(toc - tic)