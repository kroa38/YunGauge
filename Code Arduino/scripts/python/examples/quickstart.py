# -*- coding: utf-8 -*-


import httplib2
import pprint
import json

from apiclient.discovery import build
from apiclient.http import MediaFileUpload
from oauth2client.client import OAuth2WebServerFlow
from oauth2client.client import flow_from_clientsecrets


# Copy your credentials from the console
CLIENT_ID = '4464646467m2bea979krsmg7.apps.googleusercontent.com'
CLIENT_SECRET = '66666'

# Check https://developers.google.com/drive/scopes for all available scopes
OAUTH_SCOPE = 'https://www.googleapis.com/auth/drive'
scope = 'https://www.googleapis.com/auth/drive https://mail.google.com https://spreadsheets.google.com/feeds '
# Redirect URI for installed apps
REDIRECT_URI = 'urn:ietf:wg:oauth:2.0:oob'

# Path to the file to upload
FILENAME = 'document.txt'

# Run through the OAuth flow and retrieve credentials
    # Load the private key associated with the Google service account
with open('client_secrets.json') as json_file:
        json_data = json.load(json_file)

flow = OAuth2WebServerFlow(client_id=json_data['installed']['client_id'], client_secret=json_data['installed']['client_secret'],
                           scope=scope, redirect_uri=json_data['installed']['redirect_uris'])
                           
authorize_url = flow.step1_get_authorize_url()
print 'Go to the following link in your browser: ' + authorize_url
code = raw_input('Enter verification code: ').strip()
credentials = flow.step2_exchange(code)

# Create an httplib2.Http object and authorize it with our credentials
http = httplib2.Http()
http = credentials.authorize(http)

drive_service = build('drive', 'v2', http=http)

# Insert a file
media_body = MediaFileUpload(FILENAME, mimetype='text/plain', resumable=True)
body = {
  'title': 'My document',
  'description': 'A test document',
  'mimeType': 'text/plain'
}

file = drive_service.files().insert(body=body, media_body=media_body).execute()
pprint.pprint(file)