# Arrange benchmark data into table

fromLevel = 0
toLevel = 193

def main():
    table = []
    for i in range(fromLevel, toLevel + 1):
        with open('times/time%d' % i) as f:
            time = f.read().strip()
            #print '%d -> %lf' % (i, float(time))
            table += [float(time)]
            
    print '\t'.join(str(x) for x in table)

if __name__ == '__main__':
    main()
