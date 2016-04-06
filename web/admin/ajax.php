<?php
	header('Content-Type: application/json; charset=utf-8');
	require_once('db_web_config.php');
	
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
	
	$data = [];
	
	if ($mysqli && isset($_GET)) {
		if (isset($_GET['a'])) {
			switch ($_GET['a']) {
				case 'accesslog':
					$sane = sanitizePaginationParameters($mysqli, $_GET);
					$data = getAccessLog($mysqli, $sane['page'], $sane['rows'], $sane['sortindex'], $sane['sortorder']);
					break;
				case 'keys':
					$sane = sanitizePaginationParameters($mysqli, $_GET);
					$data = getKeys($mysqli, $sane['page'], $sane['rows'], $sane['sortindex'], $sane['sortorder']);
					break;
				case 'members':
					$sane = sanitizePaginationParameters($mysqli, $_GET);
					$data = getMembers($mysqli, $sane['page'], $sane['rows'], $sane['sortindex'], $sane['sortorder']);
					break;
				case 'readers':
					$sane = sanitizePaginationParameters($mysqli, $_GET);
					$data = getReaders($mysqli, $sane['page'], $sane['rows'], $sane['sortindex'], $sane['sortorder']);
					break;
				case 'allmembers':
					$data = getAllMembers($mysqli);
					break;
				case 'editmember':
					$sane = sanitizeEditMemberParameters($mysqli, $_POST);

					if ($sane['oper'] == 'add') {
						$data = addMember($mysqli, $sane);
					} else if ($sane['oper'] == 'edit') {
						$data = editMember($mysqli, $sane);
					}
					
					break;
				case 'editkey':
					$sane = sanitizeEditKeyParameters($mysqli, $_POST);
					
					if ($sane['oper'] == 'edit') {
						$data = editKey($mysqli, $sane);
					}
					
					break;
					
				default:
					break;
			}
		}
	}
	
	$mysqli->close();
	echo json_encode($data);
	
	function sanitizeEditKeyParameters($sqlclient, $d) {
		$id = $sqlclient->real_escape_string($d['id']);
		$ownerid = $sqlclient->real_escape_string($d['ownerid']);
		$ownername = $sqlclient->real_escape_string($d['ownername']);
		$ktype = $sqlclient->real_escape_string($d['ktype']);
		$active = intval($sqlclient->real_escape_string($d['active']));
		$oper = $d['oper'];
		
		if ($active != 1 && $active != 0) {
			$active = 0;
		}
		
		$retdata = [];
		
		if ($oper == 'edit') {
			$retdata['id'] = intval($id);
		} else {
			exit;
		}
		
		$retdata['oper'] = trim($oper);
		$retdata['id'] = trim($id);
		
		
		if (isset($ownerid)) {
			if (strlen($ownerid) == 0) {
				$ownerid = 'NULL';
			}
		} else {
			$ownerid = 'NULL';
		}
		
		$retdata['ownerid'] = $ownerid;
		$retdata['ownername'] = trim($ownername);
		$retdata['ktype'] = trim($ktype);
		$retdata['active'] = $active;
		print_r($retdata);
		
		return $retdata;
	}
	
	function sanitizeEditMemberParameters($sqlclient, $d) {
		$id = $sqlclient->real_escape_string($d['id']);
		$mid = $sqlclient->real_escape_string($d['mid']);
		$mname = $sqlclient->real_escape_string($d['mname']);
		$email = $sqlclient->real_escape_string($d['email']);
		$phonenumber = $sqlclient->real_escape_string($d['phonenumber']);
		$active = intval($sqlclient->real_escape_string($d['active']));
		$oper = $d['oper'];
		
		if ($active != 1 && $active != 0) {
			$active = 0;
		}
		
		if ($oper != 'add' && $oper != 'edit') {
			exit;
		}
		
		$retdata = [];
		
		if ($oper == 'edit') {
			$retdata['id'] = intval($id);
		}
		
		$retdata['oper'] = trim($oper);
		$retdata['mid'] = trim($mid);
		$retdata['mname'] = trim($mname);
		$retdata['email'] = trim($email);
		$retdata['phonenumber'] = trim($phonenumber);
		$retdata['active'] = $active;
		
		return $retdata;
	}
	
	function sanitizePaginationParameters($sqlclient, $d) {
		$page = intval($sqlclient->real_escape_string($d['page']));
		$page = max($page, 0);
		
		$rows = intval($sqlclient->real_escape_string($d['rows']));
		$rows = max($rows, 1);
		$rows = min($rows, 100);
		
		$sortindex = $sqlclient->real_escape_string($d['sidx']);
		if ($sortindex == null || $sortindex == '') {
			$sortindex = 'id';
		}

		$sortorder = strtoupper($sqlclient->real_escape_string($d['sord']));
		if ($sortorder != 'ASC' && $sortorder != 'DESC') {
			$sortorder = 'ASC';
		}
		
		$retdata = [];
		$retdata['page'] = $page;
		$retdata['rows'] = $rows;
		$retdata['sortorder'] = $sortorder;
		$retdata['sortindex'] = $sortindex;
		
		return $retdata;
	}
	
	function addMember($sqlclient, $data) {

		if ($result = $sqlclient->query(
			"INSERT INTO `members` (mid, mname, email, phonenumber, active) ".
			"VALUES ('".$data['mid']."', '".$data['mname']."', '".$data['email'].
			"', '".$data['phonenumber']."', ".$data['active'].")"
		)) {

			return true;
		}
		return false;
	}
	
	function editMember($sqlclient, $data) {
		if ($result = $sqlclient->query(
			"UPDATE `members` SET mid='".$data['mid']."', ".
			"mname='".$data['mname']."', email='".$data['email']."', ".
			"phonenumber='".$data['phonenumber']."', active=".$data['active']." ".
			"WHERE id=".$data['id']
		)) {
			return true;
		}
		
		return false;
	}
	
	function editKey($sqlclient, $data) {
		if ($result = $sqlclient->query(
			"UPDATE `keys` SET ownerid=".$data['ownerid'].", ".
			"ktype='".$data['ktype']."', active=".$data['active']." ".
			"WHERE id=".$data['id']
		)) {
			return true;
		}
		echo "ERR: " . $sqlclient->error;
		return false;
	}
	
	function getAccessLog($sqlclient, $page, $rows, $sortindex, $sortorder) {
		$retdata = [];
		if ($result = $sqlclient->query(
			"SELECT COUNT(*) AS count FROM `accesses`"
		)) {
			$r = $result->fetch_assoc();
			$result->close();
			
			$retdata['total'] = $r['count'] > 0?ceil($r['count']/$rows):0;

			if ($page > $retdata['total']) $page=$retdata['total'];
			$start = $rows * ($page - 1);

			$retdata['page'] = $page;
			$retdata['records'] = $r['count'];
			$retdata['rows'] = [];
		
			if ($result = $sqlclient->query(
					"SELECT * FROM `accesses` ORDER BY $sortindex $sortorder LIMIT $start, $rows"
				)) {
			
				while ($r = $result->fetch_assoc()) {
					$datarow = [];
					$datarow['id'] = $r['id'];
					$datarow['cell'] = $r;
					$retdata['rows'][] = $datarow;
				}
			
				$result->close();
				return $retdata;
			} else {
				return null;
			}
		
		} else {
			return null;
		}
	}
	
	function getAllMembers($sqlclient) {
		$retdata = [];
		$retdata['rows'] = [];
		if ($result = $sqlclient->query(
			"SELECT id, mname, active FROM `members` ORDER BY active DESC, mname ASC"
		)) {
			while ($r = $result->fetch_assoc()) {
				$datarow = [];
				$datarow['id'] = $r['id'];
				
				if ($r['active'] == 1) {
					$datarow['mname'] = $r['mname'];
				} else {
					$datarow['mname'] = $r['mname'] . " (Not Active)";
				
				}
				
				$datarow['active'] = $r['active'];
				$retdata['rows'][] = $datarow;
			}
		}

		return $retdata;
	}

	function getReaders($sqlclient, $page, $rows, $sortindex, $sortorder) {
		$retdata = [];
		if ($result = $sqlclient->query(
			"SELECT COUNT(*) AS count FROM `readers`"
		)) {
			$r = $result->fetch_assoc();
			$result->close();
			
			$retdata['total'] = $r['count'] > 0?ceil($r['count']/$rows):0;

			if ($page > $retdata['total']) $page=$retdata['total'];
			$start = $rows * ($page - 1);

			$retdata['page'] = $page;
			$retdata['records'] = $r['count'];
			$retdata['rows'] = [];
		
			if ($result = $sqlclient->query(
					"SELECT * FROM `readers` ORDER BY $sortindex $sortorder LIMIT $start, $rows"
				)) {
			
				while ($r = $result->fetch_assoc()) {
					$datarow = [];
					$datarow['id'] = $r['id'];
					$datarow['cell'] = $r;
					$retdata['rows'][] = $datarow;
				}
			
				$result->close();
				return $retdata;
			} else {
				return null;
			}
		
		} else {
			echo $sqlclient->error;
			return null;
		}
	}
	
	function getKeys($sqlclient, $page, $rows, $sortindex, $sortorder) {
		$retdata = [];
		if ($result = $sqlclient->query(
			"SELECT COUNT(*) AS count FROM `keys`"
		)) {
			$r = $result->fetch_assoc();
			$result->close();
			
			$retdata['total'] = $r['count'] > 0?ceil($r['count']/$rows):0;

			if ($page > $retdata['total']) $page=$retdata['total'];
			$start = $rows * ($page - 1);

			$retdata['page'] = $page;
			$retdata['records'] = $r['count'];
			$retdata['rows'] = [];
		
			if ($result = $sqlclient->query(
					"SELECT * FROM `keys` ORDER BY $sortindex $sortorder LIMIT $start, $rows"
				)) {
			
				while ($r = $result->fetch_assoc()) {
					$datarow = [];
					$datarow['id'] = $r['id'];
					$datarow['cell'] = $r;
					$retdata['rows'][] = $datarow;
				}
			
				$result->close();
				return $retdata;
			} else {
				return null;
			}
		
		} else {
			return null;
		}
	}
	
	function getMembers($sqlclient, $page, $rows, $sortindex, $sortorder) {
		$retdata = [];
		if ($result = $sqlclient->query(
			"SELECT COUNT(*) AS count FROM `members`"
		)) {
			$r = $result->fetch_assoc();
			$result->close();
			
			$retdata['total'] = $r['count'] > 0?ceil($r['count']/$rows):0;

			if ($page > $retdata['total']) $page=$retdata['total'];
			$start = $rows * ($page - 1);

			$retdata['page'] = $page;
			$retdata['records'] = $r['count'];
			$retdata['rows'] = [];
		
			if ($result = $sqlclient->query(
					"SELECT * FROM `members` ORDER BY $sortindex $sortorder LIMIT $start, $rows"
				)) {
			
				while ($r = $result->fetch_assoc()) {
					$datarow = [];
					$datarow['id'] = $r['id'];
					$datarow['cell'] = $r;
					$retdata['rows'][] = $datarow;
				}
			
				$result->close();
				return $retdata;
			} else {
				return null;
			}
		
		} else {
			return null;
		}
	}	
?>