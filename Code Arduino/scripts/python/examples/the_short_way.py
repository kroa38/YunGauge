
from apiclient.discovery import build
from oauth2client.file import Storage
from oauth2client.client import SignedJwtAssertionCredentials
import httplib2
import json

pcks12_file = 'yuno_pcks12.json'
DRIVE_SCOPE = 'https://www.googleapis.com/auth/drive'

http = httplib2.Http()
storage = Storage('analytics.dat')
credentials = storage.get()

if credentials is None or credentials.invalid:
    with open(pcks12_file) as json_file:
        json_data = json.load(json_file)
    credentials = SignedJwtAssertionCredentials(json_data['client_email'], json_data['private_key'], DRIVE_SCOPE)
    print "credential regenerated"
    storage.put(credentials)
else:
    credentials.refresh(http)
    print "credential refreshed"

http = credentials.authorize(http)
service = build(serviceName='drive', version='v2', http=http)

