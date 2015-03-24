#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
from cloudscope import get_json_data_from_file
from cloudscope import log_error

def main(argv):
    """  cette fonction retourne un paramètre en fonction l'argument passé en entrée
    elle est utilisé par l'arduino pour lire sa config en terme
    d'echantillonnage de la teleinfo et de mise à l'heure de la rtc via internet
    :itype : argv
    :rtype : int
    """

    if str(argv[0]) == "sampling_interval":
        data = get_json_data_from_file("config.json")
        print data['arduino_config']['sampling_interval']
        exit()
    if str(argv[0]) == "adjust_rtc":
        data = get_json_data_from_file("config.json")
        print data['arduino_config']['adjust_rtc']
        exit()
    else:
        print 0
        log_error("Mauvais argument passé à config.py")
        exit()
# --------------------------------------------------------------------------------------------------

if __name__ == '__main__':

    main(sys.argv[1:])