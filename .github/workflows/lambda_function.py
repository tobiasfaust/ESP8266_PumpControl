import urllib.parse
import boto3

print('Loading function')

ressource = boto3.resource('s3')
    

def lambda_handler(event, context):
    

    #bucketname = "tfa-releases"
    #targetPath = "ESP8266_PumpControl/" 
    
    bucketname = event['Records'][0]['s3']['bucket']['name']
    key = urllib.parse.unquote_plus(event['Records'][0]['s3']['object']['key'], encoding='utf-8')
    
    releaseJSON = "releases.json"
    
    bucket = ressource.Bucket(bucketname)
    path = key.split("/")
    path.pop()
    targetPath = "/".join(path)
    
    #check for existing JSON file and delete
    try:
        bucket.Object(targetPath + "/" + releaseJSON).delete()
    except Exception as e:
        print(e)

    # Generate JSON by concat all existing JSONs
    myJSON = "[ \n"
    for obj in bucket.objects.filter(Prefix=targetPath + "/"):
        if obj.key.endswith('.json'):
            file_content = obj.get()['Body'].read().decode('utf-8')
            myJSON += file_content
            
    myJSON += "]"
    
    #print(myJSON)
    
    # Put JSON to S3
    object = ressource.Object('tfa-releases', targetPath + "/" + releaseJSON)
    object.put(Body=myJSON)

    # Enable public Access
    object_acl = ressource.ObjectAcl('tfa-releases', targetPath + "/" + releaseJSON)
    response = object_acl.put(ACL='public-read')
    