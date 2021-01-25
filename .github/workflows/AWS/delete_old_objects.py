import boto3

s3 = boto3.client('s3')

get_last_modified = lambda obj: int(obj['LastModified'].strftime('%Y%m%d%H%M%S'))
save_versions = 4

def delete_old_objects(bucketname, targetpath):
    resp = s3.list_objects(Bucket=bucketname,Prefix=targetpath + "/")
    if 'Contents' in resp:
        objs = resp['Contents']
        files = sorted(objs, key=get_last_modified, reverse=True)
        
        hashtable = {}
        hashtable = {'ESP32': {'DEV':[],'PRE':[],'PROD':[]}, 
                     'ESP8266': {'DEV':[],'PRE':[],'PROD':[]}
                    }
        delete = True
        
        print ("HashTable Length: #"+str(len(hashtable["ESP32"]["DEV"])))
        
        for key in files:
            print(key)
            
            for arch in hashtable.keys():
                delete = True
                for stage in hashtable[arch].keys():
                    #print(stage)
                    if key['Key'].find("."+arch+".") > 0 and key['Key'].find("."+stage+".") > 0 :
                        if len(hashtable[arch][stage]) <= ((save_versions * 2) - 1) : # 4 Binaries + 4 Json (incl. dem jetzt kommenden)
                            hashtable[arch][stage].append(key)
                            delete = False;
                            print ("Save Object #"+str(len(hashtable[arch][stage]))+" : " + key['Key'])
                        #else :
                            #print("Delete this Object: " + key['Key'])
            
            if delete == True:
                print("Delete this Object: " + key['Key'])
                s3.delete_object(Bucket=bucketname, Key=key['Key'])   
                