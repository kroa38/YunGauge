# -*- coding: utf-8 -*- #

import urllib2
import json

# --------------------------------------------------------------------------------------------------------
CLIENT_SECRET_FILE = 'client_secrets.json'
OAUTH_CODE_FILE = 'oauth_code.json'
TOKEN_FILE = 'token_code.json'
URL = 'https://accounts.google.com/o/oauth2/device/code'
URL_ENDPOINT = 'https://www.googleapis.com/oauth2/v3/token'
GMAIL_SCOPE = 'email profile'
DRIVE_SCOPE = 'https://www.googleapis.com/auth/drive'
TEST_SCOPE ='drive profile'
# --------------------------------------------------------------------------------------------------------


def get_oauth_user_code(url, data):
    """ Return the object data from the first call to google oauth.
    :rtype : object
    """
    try:
        req = urllib2.Request(url, data)
        json_data = urllib2.urlopen(req).read()
    except:
        return 0
    else:
        f = open(OAUTH_CODE_FILE, "w")
        f.write(json_data)
        f.close()
        pyt_data = json.loads(json_data)
        return pyt_data

# --------------------------------------------------------------------------------------------------------


def poll_oauth(url, data):
    """ Return the object data from the poll call to google oauth.
    :rtype : object
    """
    try:
        req = urllib2.Request(url, data)
        json_data = urllib2.urlopen(req).read()
    except:
        return 0
    else:
        f = open(TOKEN_FILE, "w")
        f.write(json_data)
        f.close()
        pyt_data = json.loads(json_data)
        return pyt_data

# --------------------------------------------------------------------------------------------------------


def get_json_data_from_file(file_name):
    """ Return the data object from the  json file in a python object. """
    try:
        json_data = open(file_name).read()
    except:
        return 0
    else:
        python_data = json.loads(json_data)
        return python_data
# --------------------------------------------------------------------------------------------------------


if __name__ == '__main__':

    client_secret_object = get_json_data_from_file(CLIENT_SECRET_FILE)
    CLIENT_ID = client_secret_object['installed']['client_id']
    CLIENT_SECRET = client_secret_object['installed']['client_secret']
    print "client id = ", CLIENT_ID
    print "client secret = ", CLIENT_SECRET

    data_for_url = "client_id=" + CLIENT_ID + "&scope=" + DRIVE_SCOPE
    print "data for url = ",data_for_url
    user_code_object = get_oauth_user_code(URL, data_for_url)
    DEVICE_CODE = user_code_object["device_code"]
    VERIFICATION_URL = user_code_object["verification_url"]
    USER_CODE = user_code_object["user_code"]

    print "device code = ", DEVICE_CODE
    print "verification url = ", VERIFICATION_URL
    print "user code = ", USER_CODE

    data_for_url = "client_id=" + CLIENT_ID + "&client_secret=" + CLIENT_SECRET + "&code=" + DEVICE_CODE +\
                   "&grant_type=http://oauth.net/grant_type/device/1.0"

    print data_for_url
    person = input('go to web and type the user code')
    access_token_object = poll_oauth(URL_ENDPOINT, data_for_url)

    if access_token_object != 0 :
        token_object =  get_json_data_from_file(TOKEN_FILE)
        ACCESS_TOKEN = token_object["access_token"]
        TOKEN_TYPE = token_object["token_type"]
        EXPIRES_IN = token_object["expires_in"]
        REFRESH_TOKEN = token_object["refresh_token"]
        ID_TOKEN = token_object["id_token"]
        print "access_token = ", ACCESS_TOKEN
        print "token_type = ", TOKEN_TYPE
        print "expires_in = ", EXPIRES_IN
        print "refresh_token = ", REFRESH_TOKEN
        print "id_token = ", ID_TOKEN


