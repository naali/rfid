<?php
	require_once("db_rpi_config.php");
	
	function updateLastSeen($sqlclient, $readername) {
		$result = $sqlclient->query(
			"INSERT INTO `readers` (readername, ts_created) VALUES ('$readername', CURRENT_TIMESTAMP) ".
			"ON DUPLICATE KEY UPDATE ts_lastseen=CURRENT_TIMESTAMP"
		);
	}

	function findKey($sqlclient, $keyhash, $accessreq, $pincode) {
		$keyhash = $sqlclient->real_escape_string($keyhash);
		$accessreq = $sqlclient->real_escape_string($accessreq);
		$pincode = $sqlclient->real_escape_string($pincode);
		
		updateLastSeen($sqlclient, $accessreq);

		if ($result = $sqlclient->query(
				"INSERT INTO `keys` (hash, ts_created) SELECT * FROM (SELECT '$keyhash', CURRENT_TIMESTAMP) AS tmp ".
				"WHERE NOT EXISTS (SELECT hash FROM `keys` WHERE hash='$keyhash') LIMIT 1")) {
				
			if ($result = $sqlclient->query(
					"SELECT `keys`.id AS keysid, `keys`.hash AS keyshash, `keys`.active AS keysactive, ".
					"`keys`.ownerid AS keysownerid, `keys`.ktype AS keysktype, ".
					"`members`.id AS membersid, `members`.active AS membersactive, `members`.mname AS membersmname, ".
					"`members`.pincode AS pincode, ".
					"`readers`.require_pin as require_pin, `readers`.readername as readername ".
					"FROM `keys`, `members`, `readers` WHERE ".
					"`keys`.hash='$keyhash' AND `keys`.active = 1 AND `keys`.ownerid IS NOT NULL AND ".
					"`keys`.ownerid = `members`.id AND `members`.active = 1 AND ".
					"(`keys`.ktype = 'master' OR `keys`.ktype = 'member' OR `keys`.ktype = 'guest') AND ".
					"(`readers`.readername = '$accessreq' AND (".
					"(`readers`.require_pin = 1 AND `members`.pincode = '$pincode' AND `members`.pincode IS NOT NULL AND `members`.pincode != '') OR ".
					"(`readers`.require_pin = 0)))"
					)) {
			
				if ($result->num_rows == 1) {
					$granted = 0;
					$r = $result->fetch_assoc();
					$keysactive = $r['keysactive'];
					$keysktype = $r['keysktype'];
					$keyshash = $r['keyshash'];
					$keysownerid = $r['keysownerid'];

					$membersmname = $r['membersmname'];
					$membersactive = $r['membersactive'];
					$membersid = $r['membersid'];
					
					// In theory unnecessary as all these are already checked in the db query
					if ($keysactive == 1 && $membersactive == 1 && 
						$keysownerid == $membersid &&
						($keysktype == 'master' || $keysktype == 'member' || $keysktype == 'guest')) {
						$granted = 1;
					}
					
					if ($result = $sqlclient->query(
							"INSERT INTO `accesses` (ownername, ownerid, hash, ktype, active, request, agranted)".
							" VALUES ('$membersmname', $membersid, '$keyshash', '$keysktype', $keysactive, '$accessreq', $granted)"
						)) {
						// Everything went smoothly
					} else {
						// Log the access error?
					}

					return ($granted == 1); 
				} else {
					if ($keyres = $sqlclient->query("SELECT * FROM `keys` WHERE `keys`.hash='$keyhash'")) {
						$r = $keyres->fetch_assoc();
						$granted = 0;
						$keysactive = $r['active'];
						$keysktype = $r['ktype'];
						$keyshash = $r['hash'];
						$keysownerid = $r['ownerid'];
						$membersname = 'Not assigned';
						$membersid = 'NULL';
						
						if (isset($keysownerid) && $keysownerid != NULL) {
							if ($memberres = $sqlclient->query("SELECT * FROM `members` WHERE `members`.id=$keysownerid")) {
								$r = $memberres->fetch_assoc();
								$membersname = $r['mname'];
								$membersid = $r['id'];
							}
						} else {
							$membersid = 'NULL';
							$keysownerid = 'NULL';
						}
						
						$result = $sqlclient->query(
							"INSERT INTO `accesses` (ownername, ownerid, hash, ktype, active, request, agranted) ".
							"VALUES ('$membersname', $membersid, '$keyshash', '$keysktype', $keysactive, '$accessreq', $granted)"
						);
					}
				
					return false;
				}
			}
		}
		
		return false;
	}

	function connectDatabase($db_addr, $db_user, $db_pass, $db_name) {
		$sqlclient = new mysqli($db_addr, $db_user, $db_pass, $db_name);
		if ($sqlclient->connect_errno) {
		    echo "Failed to connect to MySQL: (" . $mysqli->connect_errno . ") " . $mysqli->connect_error;
		    exit;
		}
		
		$sqlclient->set_charset("utf8");
		return $sqlclient;
	}
	
	$mysqli = connectDatabase($db_addr, $db_user, $db_pass, $db_name);
	
	if ($mysqli) {

		$keyhash = '';
		$accessreq = '';
	
		if (isset($_POST) && isset($_POST['k']) && isset($_POST['a']) && isset($_POST['p'])) {
			$keyhash = $_POST['k'];
			$accessreq = $_POST['a'];
			$pincode = $_POST['p'];
			
		} else if (isset($_GET) && isset($_GET['k']) && isset($_GET['a']) && isset($_GET['p'])) {
			$keyhash = $_GET['k'];
			$accessreq = $_GET['a'];
			$pincode = $_GET['p'];

		} else {
			http_response_code(404);
			echo "ummm, no?\n";
			exit;
		}
	
		if (findKey($mysqli, $keyhash, $accessreq, $pincode)) {
			http_response_code(200);
			header('Keyhash: ' . md5("Kulosaari" . $keyhash . $accessreq . "00570") );
		} else {
			http_response_code(404);
			echo "ummm, no?\n";
		}
	
		$mysqli->close();
	}  else {
		http_response_code(404);
		echo "ummm, no?\n";
	}
?>