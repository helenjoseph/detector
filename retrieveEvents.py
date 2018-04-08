import os, sys, requests, json

def call2Webservice(entity):
    url= "http://localhost:8080/SensorThingsServer-1.0/v1.0/"+entity
    headers ={"Content-Type": "application/json", "Accept": "application/json"}
    request = requests.get(url, headers = headers)
    data = request.json()
    data.items()
    val = data.values()
    return val

def readNextLink(url):
    headers ={"Content-Type": "application/json", "Accept": "application/json"}
    request = requests.get(url, headers = headers)
    data = request.json()
    data.items()
    val = data.values()
    return val

def getID(data):
    DS_id = [i['@iot.id'] for i in data]
    #p_time = [i['phenomenonTime'] for i in data]
    #name = [i['name'] for i in data]
    return DS_id
def getName(data):
    name = [i['name'] for i in data]
    return name
def getPTime(data):
    p_time = [i['phenomenonTime'] for i in data]
    return p_time


eId = 0
eBand = 0
eTime = 0

bLoop = False
counter = 0

'''
print "Provide Time period as YYYY-MM-DDTHH:MM:SSZ"
sTime = raw_input("Start Time:")
eTime = raw_input("End Time:")
'''
print "Provide Time period as YYYY-MM-DD"
sTime = raw_input("Start Time:")
eTime = raw_input("End Time:")
sID = raw_input("Enter the start ID:")
eID = raw_input("Enter the end ID:")
startTime = sTime+"T00:00:00Z"
endTime = eTime+"T23:59:59Z"
queryParameter = "Datastreams?$filter = phenomenonTime ge "+startTime+" and phenomenonTime le "+endTime+"&$select=id,phenomenonTime,name"
info = call2Webservice(queryParameter)

total = info[0]

while(bLoop == False):
    if counter == 0:
        if 'http' in info[1]:
            counter += 1
            correctLink = info[1].replace("localhost", "127.0.0.1")
            eID = getID(info[2])
            eBand = getName(info[2])

            eTime = getPTime(info[2])
        else:
            eID = getID(info[1])
            eBand = getName(info[1])

            eTime = getPTime(info[1])

            bLoop = True
    if(counter>0):
        info = readNextLink(correctLink)
        if 'http' in info[1]:
            correctLink = info[1].replace("localhost", "127.0.0.1")
            eID.extend(getID(info[2]))
            eBand.extend(getName(info[2]))

            eTime.extend(getPTime(info[2]))
        else:
            eID.extend(getID(info[1]))
            eBand.extend(getName(info[1]))

            eTime.extend(getPTime(info[1]))    #

            counter = 0
            bLoop = True

print "\nTotal number of datastreams:" + str(total) +"\n"

print "\nID \t Band \t\t\t\t\t Phenomenon Time \n"

'''
for i in range(total):
    print str(eID[i]) + "\t" + eBand[i] + "\n"
'''
for i in range(total):
    print str(eID[i]) + "\t" + eBand[i] + "\t" + eTime[i] +"\n"
