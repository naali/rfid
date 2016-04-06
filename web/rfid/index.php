<?php
	require_once("db_rpi_config.php");
	
	function updateLastSeen($sqlclient, $readername) {
		$sqlclient->query(
			"INSERT INTO `readers` (readername) VALUES ('$readername') ".
			"ON DUPLICATE KEY UPDATE ts_lastseen=CURRENT_TIMESTAMP"
		);
	}

	function findKey($sqlclient, $keyhash, $accessreq) {
		$keyhash = $sqlclient->real_escape_string($keyhash);
		$accessreq = $sqlclient->real_escape_string($accessreq);
		
		updateLastSeen($sqlclient, $accessreq);

		if ($result = $sqlclient->query(
				"INSERT INTO `keys` (hash) SELECT * FROM (SELECT '$keyhash') AS tmp ".
				"WHERE NOT EXISTS (SELECT hash FROM `keys` WHERE hash='$keyhash') LIMIT 1")) {
				
			if ($result = $sqlclient->query(
					"SELECT `keys`.id AS keysid, `keys`.hash AS keyshash, `keys`.active AS keysactive, ".
					"`keys`.ownerid AS keysownerid, `keys`.ktype AS keysktype, ".
					"`members`.id AS membersid, `members`.active AS membersactive, `members`.mname AS membersmname ".
					"FROM `keys`, `members` WHERE ".
					"`keys`.hash='$keyhash' AND `keys`.active = 1 AND `keys`.ownerid IS NOT NULL AND ".
					"`keys`.ownerid = `members`.id AND `members`.active = 1 AND ".
					"(`keys`.ktype = 'master' OR `keys`.ktype = 'member' OR `keys`.ktype = 'guest')" )) {
			
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
						// Log the access log error?
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
							if ($memberres = $sqlclient->query("SELECT * FROM `members` WHERE `members`.id=$ownerid")) {
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
	
		if (isset($_POST) && isset($_POST['k']) && isset($_POST['a'])) {
			$keyhash = $_POST['k'];
			$accessreq = $_POST['a'];
			
		} else if (isset($_GET) && isset($_GET['k']) && isset($_GET['a'])) {
			$keyhash = $_GET['k'];
			$accessreq = $_GET['a'];

		} else {
			http_response_code(404);
			echo "ummm, no?\n";
			exit;
		}
	
		if (findKey($mysqli, $keyhash, $accessreq)) {
			http_response_code(200);
			header('Keyhash: ' . md5("SOMETHING1" . $keyhash . $accessreq . "SOMETHING2") );
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