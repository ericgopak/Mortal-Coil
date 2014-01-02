import httplib, urllib, sys

with open('credentials.txt') as creds:
    USERNAME, PASSWORD = tuple(creds.read().split())

def main(argv):
    with open('output.txt', 'r') as FILE:
        data = FILE.read().split()
    #print data

    params = urllib.urlencode({'name': USERNAME, 'password': PASSWORD, 'x': data[0], 'y': data[1], 'path': data[2]})
    headers = {'Content-type': 'application/x-www-form-urlencoded', 'Accept': 'text/plain'}

    # Sending HTTP request to the server
    connection = httplib.HTTPConnection('www.hacker.org')
    connection.request('POST', '/coil/index.php', params, headers)

    r = connection.getresponse()

    print r.status, r.reason

    data = r.read()
    print data

    connection.close()

if __name__ == '__main__':
    main(sys.argv)
