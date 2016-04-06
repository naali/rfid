#!/bin/sh

TEMPLATEFILE=create_database.sql.template
TARGETFILE=`echo $TEMPLATEFILE | sed "s/\.template//"`

echo "Template: $TEMPLATEFILE"
echo "Target: $TARGETFILE"

if [ -f $TARGETFILE ]; then
	echo "Targetfile already exists, exiting!"
	exit;
fi

RFID_RPI_PASSWD=`cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 16 | head -n 1`
RFID_WEB_PASSWD=`cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 16 | head -n 1`
RFID_ADMIN_PASSWD=`cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 16 | head -n 1`

cat $TEMPLATEFILE | \
sed "s/RFID_RPI_PASSWD/$RFID_RPI_PASSWD/" | \
sed "s/RFID_WEB_PASSWD/$RFID_WEB_PASSWD/" | \
sed "s/RFID_ADMIN_PASSWD/$RFID_ADMIN_PASSWD/" > $TARGETFILE

