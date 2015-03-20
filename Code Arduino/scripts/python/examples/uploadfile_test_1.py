#!/usr/bin/python -u
#
# Google Docs file upload tool
# (c)oded 2012 http://planzero.org/
#
# Requirements:
# gdata >= 2.0.15  http://code.google.com/p/gdata-python-client/
# magic >= 0.1     https://github.com/ahupp/python-magic (Note: this is NOT the
#                  same as the python-magic Ubuntu package, which is older!)
#

# Imports
# this exemple upload a file to google drive
#
from __future__ import division
from mimetypes import MimeTypes
import urllib 
import sys, time, os.path, atom.data, gdata.client, gdata.docs.client, gdata.docs.data

# Settings
username = 'email'
password = 'password'


# Heads up
print 'Upload a file to Google Drive'

# Set the filename
filename = 'c:\\anaconda\\work\\docs_example.py'

# Open the file to be uploaded
try:
    fh = open(filename)
except IOError, e:
    sys.exit('ERROR: Unable to open ' + filename + ': ' + e[1])

# Get file size and type
file_size = os.path.getsize(fh.name)

mime = MimeTypes()
url = urllib.pathname2url(filename)
file_type = mime.guess_type(url)
print file_type
# Create a Google Docs client
docsclient = gdata.docs.client.DocsClient(source='test upload file')

# Log into Google Docs
print 'o Logging in...',
try:
    docsclient.ClientLogin(username, password, docsclient.source);
except (gdata.client.BadAuthentication, gdata.client.Error), e:
    sys.exit('ERROR: ' + str(e))
except:
    sys.exit('ERROR: Unable to login')
print 'success!'

# The default root collection URI
uri = 'https://docs.google.com/feeds/upload/create-session/default/private/full'

# Make sure Google doesn't try to do any conversion on the upload (e.g. convert images to documents)
uri += '?convert=false'

# Create an uploader and upload the file
# Hint: it should be possible to use UploadChunk() to allow display of upload statistics for large uploads
t1 = time.time()
print 'o Uploading file...',
uploader = gdata.client.ResumableUploader(docsclient, fh, file_type[0], file_size, chunk_size=1048576, desired_class=gdata.data.GDEntry)
new_entry = uploader.UploadFile(uri, entry=gdata.data.GDEntry(title=atom.data.Title(text=os.path.basename(fh.name))))
print 'success!'
print 'Uploaded', '{0:.2f}'.format(file_size / 1024 / 1024) + ' MiB in ' + str(round(time.time() - t1, 2)) + ' seconds'