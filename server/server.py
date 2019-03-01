import logging
import socket
import urllib.request
import binascii
import datetime
from pymongo import MongoClient 

external_ip = urllib.request.urlopen('https://ident.me').read().decode('utf8')

# MongoDB client and database definition
client = MongoClient('mongodb://admin:Mf41jHc7gAfLzKhZ@lisbon-pm-monitoring-shard-00-00-6b7v3.gcp.mongodb.net:27017,lisbon-pm-monitoring-shard-00-01-6b7v3.gcp.mongodb.net:27017,lisbon-pm-monitoring-shard-00-02-6b7v3.gcp.mongodb.net:27017/test?ssl=true&replicaSet=lisbon-pm-monitoring-shard-0&authSource=admin&retryWrites=true')
pmDB = client['pm']
pm = pmDB['pm']

print(external_ip)
log = logging.getLogger('udp_server')

def udp_server(host='138.68.165.208', port=5555):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    log.info("Listening on udp %s:%s" % (host, port))
    s.bind((host, port))
    while True:
        (data, addr) = s.recvfrom(128*1024)
        yield data

FORMAT_CONS = '%(asctime)s %(name)-12s %(levelname)8s\t%(message)s'
logging.basicConfig(level=logging.DEBUG, format=FORMAT_CONS)

for data in udp_server():
    # Debug
    log.debug("%s" % (data))
    print("%s" % data)

    # Parse string
    decoData = data.decode('utf-8')
    splitData = decoData.split()

    if splitData[0] == 'Tu.cNszz1)~MqSy_ok&mhZZ#FLZz8%':
        pm10 = splitData[1]
        pm2_5 = splitData[2]
        location = splitData[3]
        now = datetime.datetime.now()
        curDate = now.strftime("%Y-%m-%d %H:%M")
        
        # Save measures in database
        measure = {"date": curDate, "location": location, "pm10": pm10, "pm2_5": pm2_5}
        if not pm.find_one({"date": curDate}):
            pm.insert_one(measure)
    elif splitData[0] == 'b!PKnFniJ~jbj*yG)j`qyo,vL!2uK{':
        location = splitData[1]
        print("Sensor error at " + location)