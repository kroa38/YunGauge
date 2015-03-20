import oauth2client.client
from apiclient.discovery import build
from oauth2client.client import SignedJwtAssertionCredentials
import json
import urllib
import urllib2


drive_scope = 'https://www.googleapis.com/auth/drive'
# Settings
pcks12_file = 'yuno_pcks12.json'
token_file = 'token_code.json'


def get_access_token(scope):

    # Load the private key associated with the Google service account
    with open(pcks12_file) as json_file:
        json_data = json.load(json_file)

    # Get and sign JWT
    credential = oauth2client.client.SignedJwtAssertionCredentials(json_data['client_email'], json_data['private_key'], scope)
    jwt_complete = credential._generate_assertion()
    # Get token from server
    data = {'grant_type': 'urn:ietf:params:oauth:grant-type:jwt-bearer','assertion': jwt_complete}
    json_data = urllib2.urlopen("https://accounts.google.com/o/oauth2/token", urllib.urlencode(data)).read()
    f = open(token_file, "w")
    f.write(json_data)
    f.close()
    access_token_response = json.loads(json_data)
    return access_token_response

def service_build(scope):

    # Load the private key associated with the Google service account
    with open(pcks12_file) as json_file:
        json_data = json.load(json_file)

    # Get and sign JWT
    credentials = oauth2client.client.SignedJwtAssertionCredentials(json_data['client_email'], json_data['private_key'], scope)
    from httplib2 import Http
    http_auth = credentials.authorize(Http())
    service = build('drive', 'v2', http=http_auth)
    return service


if __name__ == '__main__':

    service_build(drive_scope)
