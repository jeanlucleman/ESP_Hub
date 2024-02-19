# https://stackoverflow.com/questions/56923895/auto-increment-build-number-using-platformio et https://gitlab.com/pvojnisek/buildnumber-for-platformio/tree/master
FILENAME_BUILDNO = 'versioning'
FILENAME_VERSION_H = 'include/version.h'
version = 'v1.1'
# 0.0: 09/01/2024 start of project 
import datetime

build_no = 0
try:
    with open(FILENAME_BUILDNO) as f:
        build_no = int(f.readline()) + 1
except:
    print('Starting build number from 1..')
    build_no = 1
with open(FILENAME_BUILDNO, 'w+') as f:
    f.write(str(build_no))
    print('Build number: {}'.format(build_no))

hf = """
#ifndef BUILD_NUMBER
  #define BUILD_NUMBER "{}"
#endif
#ifndef VERSION
  #define VERSION "{} - {}"
#endif
#ifndef VERSION_SHORT
  #define VERSION_SHORT "{}"
#endif
""".format(build_no, version+"."+str(build_no), datetime.datetime.now(), version+"."+str(build_no))
with open(FILENAME_VERSION_H, 'w+') as f:
    f.write(hf)