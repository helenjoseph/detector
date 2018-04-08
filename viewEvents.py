import numpy as np
import scipy as sy
import matplotlib as mtp
import pylab as pyl
import matplotlib.pyplot as plt
import math
from scipy import signal
from mpl_toolkits.mplot3d import Axes3D
from matplotlib.ticker import LinearLocator, FormatStrFormatter
from matplotlib import cm
from matplotlib.pyplot import specgram
# Get lighting object for shading surface plots.
from matplotlib.colors import LightSource
from matplotlib import gridspec

import os, sys, requests, json
import time
import traceback
from time import sleep

def call2Webservice(entity):
    url= "http://localhost:8080/SensorThingsServer-1.0/v1.0/"+entity
    headers ={"Content-Type": "application/json", "Accept": "application/json"}
    request = requests.get(url, headers = headers)
    data = request.json()
    data.items()
    val = data.values()
    return val

def readNextLink(url):
    #url = entity.replace("localhost", "192.168.178.90")
    headers ={"Content-Type": "application/json", "Accept": "application/json"}
    request = requests.get(url, headers = headers)
    data = request.json()
    data.items()
    val = data.values()
    return val

def readObservations(value):
    Observation = [i['result'] for i in value]
    return Observation

counter=0
bLastLoop = False
dFreq = 0
mSpectrogram = 0

FTsize=4096
STFTsize=128
spectrogram_loops = 100

try:
    dsID = raw_input("Please provide Datastream ID:")
    queryDS = "Datastreams("+dsID+")"
    query = call2Webservice(queryDS)
except ValueError:
    print "Please enter a valid Datastream ID\n"
    sys.exit(0)
else:
    activity = query[4]
    print "Plotting requested observation..."

queryParameter = "Datastreams("+dsID+")/Observations?$select=result"
info = call2Webservice(queryParameter)
#info = call2Webservice("Datastreams(347)/Observations?$select=result")
dTotal = info[0]
while(bLastLoop == False):
    if counter == 0:
        if 'http' in info[1]:
            counter += 1
            correctLink = info[1].replace("localhost", "127.0.0.1")
            dFreq = readObservations(info[2])
        else:
            dFreq = readObservations(info[1])
            bLastLoop = True
    if counter>0:
        info = readNextLink(correctLink)
        if 'http' in info[1]:
            correctLink = info[1].replace("localhost", "127.0.0.1")
            dFreq.extend(readObservations(info[2]))
        else:
            dFreq.extend(readObservations(info[1]))
            counter = 0
            bLastLoop = True

if ("Power Profile" in activity):
    #Power Profile
    fig_pow = plt.figure('Power Profile')
    for i in dFreq:
        dFreq_dB = 10*(np.log10(dFreq))
    plt.xlabel('Time [sec]')
    plt.ylabel('Power [dB]')
    plt.plot(dFreq)

if("Power Spectrum" in activity):
    #Power Spectrum
    fig_psd = plt.figure('Power Spectrum')
    dx=np.arange(-FTsize/2,FTsize/2, dtype='float')*100/FTsize
    dy = np.fft.fftshift(10*(np.log10(dFreq[2:(FTsize-2)])))
    plt.xlabel('Freq [MHz]')
    plt.ylabel('Power [dB]')
    plt.plot(dx[2:(FTsize-2)],dy)

if("Spectrogram" in activity):
    #Spectrogram
    fig_spec = plt.figure('Spectrogram')
    TimeAxis = np.arange(0,spectrogram_loops-1)*(STFTsize/100)*0.5
    StftFreqAxis = np.arange(-STFTsize/2, STFTsize/2-1, dtype='float')*100/STFTsize
    X,Y= np.meshgrid(StftFreqAxis,TimeAxis)
    Z=np.zeros((spectrogram_loops-1,STFTsize-1))
    mfreq_spectrogram = [dFreq[i:i+STFTsize] for i in range(0, len(dFreq), STFTsize)]
    for ii in range(0, spectrogram_loops-1):
        Z[ii,0:(STFTsize-1)]=np.fft.fftshift(10*(np.log10(mfreq_spectrogram[ii][0:(STFTsize-1)])))
    ax = plt.gca(projection='3d')
    ax.zaxis.set_major_locator(LinearLocator(10))
    ax.zaxis.set_major_formatter(FormatStrFormatter('%.02f'))
    ax.view_init(azim=0, elev=90)
    surf=ax.plot_surface(X,Y,Z, rstride=1, cstride=1, cmap=cm.coolwarm)
    colorbar_name=fig_spec.colorbar(surf, shrink=0.5, aspect=5, label='Power [dB]', orientation='vertical')
    plt.xlabel('Freq [MHz]')
    plt.ylabel(u'Time [\u03bcs]')
    ax.set_zlabel('Power [dB]')

plt.show()
