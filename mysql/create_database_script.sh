#!/bin/sh

SQL_TEMPLATEFILE=create_database.sql.template
SQL_TARGETFILE=`echo $SQL_TEMPLATEFILE | sed "s/\.template//"`

PHP_TEMPLATEFILE=db_config.php.template
PHP_WEB_TARGETFILE=`echo $PHP_TEMPLATEFILE | sed "s/\.template//" | sed "s/db_/db_web_/"`
PHP_RPI_TARGETFILE=`echo $PHP_TEMPLATEFILE | sed "s/\.template//" | sed "s/db_/db_rpi_/"`

echo "SQL Template: $SQL_TEMPLATEFILE"
echo "SQL Target: $SQL_TARGETFILE"
echo "PHP Template: $PHP_TEMPLATEFILE"
echo "PHP Web Target: $PHP_WEB_TARGETFILE"
echo "PHP RPI Target: $PHP_RPI_TARGETFILE"

if [ -f $SQL_TARGETFILE ]; then
	echo "Targetfile $SQL_TARGETFILE already exists, exiting!"
	exit;
fi

if [ -f $PHP_WEB_TARGETFILE ]; then
	echo "Targetfile $PHP_WEB_TARGETFILE already exists, exiting!"
	exit;
fi

if [ -f $PHP_RPI_TARGETFILE ]; then
	echo "Targetfile $PHP_RPI_TARGETFILE already exists, exiting!"
	exit;
fi

RFID_RPI_PASSWD=`cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 16 | head -n 1`
RFID_WEB_PASSWD=`cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 16 | head -n 1`
RFID_ADMIN_PASSWD=`cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 16 | head -n 1`

cat $SQL_TEMPLATEFILE | \
sed "s/RFID_RPI_PASSWD/$RFID_RPI_PASSWD/" | \
sed "s/RFID_WEB_PASSWD/$RFID_WEB_PASSWD/" | \
sed "s/RFID_ADMIN_PASSWD/$RFID_ADMIN_PASSWD/" > $SQL_TARGETFILE

cat $PHP_TEMPLATEFILE | \
sed "s/DB_USERNAME/rfid_web/" | \
sed "s/DB_PASSWORD/$RFID_WEB_PASSWD/" > $PHP_WEB_TARGETFILE

cat $PHP_TEMPLATEFILE | \
sed "s/DB_USERNAME/rfid_rpi/" | \
sed "s/DB_PASSWORD/$RFID_RPI_PASSWD/" > $PHP_RPI_TARGETFILE

