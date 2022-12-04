A feladathoz 3 fajl tartozik:
    - szerver.c 
    - kliens.c
    - stop.c

A szerver es kliens fajlok elvegzik a feladat altal kert szerver-kliens kommunikaciot.
A stop fajl az a szerver leallitasaert felelos. 

A szerver ket megallasi feltetelre is megvan irva. Az elso az amikor egy a programban definialt maximalis 
kliens kiszolgalasa utan megall. Ez jelen esetben 10-re van allitva. 
A masik megallasi feltetel a stop fajl futattasa. Abban az esetben, amikor a stop fajlt nem egyedulalloan futtatjuk,
hanem parhuzamosan a tobbi klienssel megeshet, hogy a kliensek beragadnak egy receive-nel, ezert ajanlatos a stop fajlt
egy kulonallo parancskent futtatni.

A csomagolt allomany tartalmaz egy makefilet-t amely a megfelelo fajlokat forditja le es 
kesziti el a futtathato allomanyokat. 

A programok futtatasa a paranccsorbol vegezeheto el.

Futattas pelda: ./sz 
                ./kl almafa & ./kl szerver.c & ./kl kliens.c & ./kl temp & sleep 3
                ./stop
Az output_server.txt tartalmazza a szerver altal generalt kimeneteket a fenti futattasra.
Az output_client.txt tartalmazza a kliens altal generalt kimeneteket a fenti futtatasra.