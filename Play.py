import httplib
import subprocess
import pickle
import shutil
import urllib
import time
import sys

Debug = 0
Single = 0

with open('credentials.txt') as creds:
    USERNAME, PASSWORD = tuple(creds.read().split())

PATH_TO_EXECUTABLE = 'build/msvc/MortalCoil/Release/MortalCoil.exe'
INPUT_FILENAME = 'mortal_coil.txt'

def main(argv):
    while (True):
        # Requesting HTTP data from the server via method POST
        params = urllib.urlencode({'name': USERNAME, 'password': PASSWORD})
        headers = {
            'Content-type': 'application/x-www-form-urlencoded',
            'Accept': 'text/plain'
        }
        
        connection = httplib.HTTPConnection('www.hacker.org')
        connection.request('POST', '/coil/index.php', params, headers)
        r = connection.getresponse()
        
        print r.status, r.reason
        
        data = r.read()
        connection.close()
        
        ind1 = data.find('FlashVars=')
        
        if ind1 == -1:
            print 'No FlashVars found...'
        else:
            ind2 = data.find('\"', ind1 + 11)
            if (ind2 == -1):
                print 'Bad line occured...'
            else:
                # Parsing the data
                t1 = data.find('Level:')
                t2 = data.find('<', t1 + 7)
                
                level = data[t1 + 7 : t2]
                print 'Level:', level
                
                args = data[ind1 + 11 : ind2]
                args = args.split('&')
                
                BoardW = int(args[0][2:])
                BoardH = int(args[1][2:])
                Board = args[2][6:]
                
                print 'H = %d\tW = %d' % (BoardH, BoardW)
                
                # Write data to the input file / make a backup
                filename = 'Level' + level
                with open(filename, 'w') as f:
                    f.write('%d %d\n' % (BoardH, BoardW))
                    
                    for i in range(0, int(BoardH)):
                        f.write(Board[int(BoardW) * i : int(BoardW) * (i + 1)] + '\n')
                
                shutil.copy(filename, INPUT_FILENAME)
                
                # Calling C++ program here -----------------------------------------------------------------------------------------
                startTime = time.clock()
                subprocess.call([PATH_TO_EXECUTABLE])
                endTime = time.clock()
                totalTime = endTime - startTime
                # --------------------------------------------------------------------------------------------------------------------
                
                # Reading output file
                with open('output.txt', 'r') as f:
                    res = f.read()
                    
                    # Backup the solution
                    solutionBackup = open('solution' + level, 'w')
                    solutionBackup.write(res)
                    solutionBackup.close()
                    
                    res = res.split()
                
                # Write out the elapsed time
                with open('time' + level, 'w') as f:
                    f.write(str(totalTime))
                
                if (Debug):
                    break
                
                # Sending HTTP request to the server via method POST
                params = urllib.urlencode({'name': USERNAME, 'password': PASSWORD, 'x': res[0], 'y': res[1], 'path': res[2]})
                headers = {'Content-type': 'application/x-www-form-urlencoded', 'Accept': 'text/plain'}
                
                connection = httplib.HTTPConnection('www.hacker.org')
                connection.request('POST', '/coil/index.php', params, headers)
                r = connection.getresponse()
                
                print r.status, r.reason
                connection.close()
                
                #raw_input('Press Enter to continue...')
                print
            
        if (Single):
            break

if __name__ == '__main__':
    main(sys.argv)
