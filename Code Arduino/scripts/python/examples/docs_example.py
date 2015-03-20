#!/usr/bin/python
#
# Copyright (C) 2007, 2009 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


__author__ = ('api.jfisher (Jeff Fisher), '
              'e.bidelman (Eric Bidelman)')


import sys
import re
import os.path
import getopt
import getpass
import gdata.docs.service
import gdata.spreadsheet.service

class DocsSample(object):
  """A DocsSample object demonstrates the Document List feed."""

  def __init__(self, email, password):

    source = 'Document upload test'
    self.gd_client = gdata.docs.service.DocsService()
    self.gd_client.ClientLogin(email, password, source=source)

  def _GetFileExtension(self, file_name):
    match = re.search('.*\.([a-zA-Z]{3,}$)', file_name)
    if match:
      return match.group(1).upper()
    return False

  def _UploadMenu(self):
    """Prompts that enable a user to upload a file to the Document List feed."""
    file_path = ''
    file_path = 'c:\\anaconda\\work\\testupload.csv'

    if not file_path:
      return
    elif not os.path.isfile(file_path):
      print 'Not a valid file.'
      return

    file_name = os.path.basename(file_path)
    ext = self._GetFileExtension(file_name)

    if not ext or ext not in gdata.docs.service.SUPPORTED_FILETYPES:
      print 'File type not supported. Check the file extension.'
      return
    else:
      content_type = gdata.docs.service.SUPPORTED_FILETYPES[ext]

    title = ''
    while not title:
      title = raw_input('Enter name for document: ')

    try:
      ms = gdata.MediaSource(file_path=file_path, content_type=content_type)
    except IOError:
      print 'Problems reading file. Check permissions.'
      return

    if ext in ['CSV', 'ODS', 'XLS', 'XLSX']:
      print 'Uploading spreadsheet...'
    elif ext in ['PPT', 'PPS']:
      print 'Uploading presentation...'
    else:
      print 'Uploading word processor document...'

    entry = self.gd_client.Upload(ms, title)

    if entry:
      print 'Upload successful!'
      print 'Document now accessible at:', entry.GetAlternateLink().href
    else:
      print 'Upload error.'
      
  
def main():
  """Demonstrates use of the Docs extension using the DocsSample object."""
  user = 'email'
  pw = 'password'

  try:
    sample = DocsSample(user, pw)
  except gdata.service.BadAuthentication:
    print 'Invalid user credentials given.'
    return

  sample._UploadMenu()


if __name__ == '__main__':
  main()
