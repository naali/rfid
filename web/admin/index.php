<!DOCTYPE html>
<html>
	<head>
		<meta charset="utf-8"/>
		<title>Member and Access Control Management</title>
		<link href="jquery-ui-1.11.4.custom/jquery-ui.min.css"  rel="stylesheet" type="text/css"/>
		<link href="admin.css" rel="stylesheet" rel="stylesheet" type="text/css"/>
		<link href="css/ui.jqgrid.css" rel="stylesheet" type="text/css"/>

		<script src="jquery-1.12.2.min.js"></script>
		<script src="jquery-ui-1.11.4.custom/jquery-ui.min.js"></script>		
		
		<script src="js/i18n/grid.locale-en.js"></script>
		<script src="js/jquery.jqGrid.min.js"></script>
		<script>var global = {};</script>
		<script src="admin.js"></script>
		<script>$(document).ready(function() { initialize(); }); </script>
	</head>
	<body>
		<div id="menutabs">
			<ul>
				<li><a href="#menutab-frontpage">Frontpage</a></li>
				<li><a href="#menutab-members">Members</a></li>
				<li><a href="#menutab-keys">Keys</a></li>
				<li><a href="#menutab-accesslog">Access Log</a></li>
				<li><a href="#menutab-readers">Readers</a></li>
			</ul>
			<div id="menutab-frontpage">
				<p>Member and Access Control Management</p>
				<ul>
					<li>A key will only function if it's type is not Lost or Unknown, it is set Active, has been assigned to a member and that member is set Active.</li>
					<li>Access log is what it says, it shows whether an access was granted at the time.</li>
				</ul>
			</div>
			<div id="menutab-members">
				<table id="members"></table>
			    <div id="members_pager"></div> 
			</div>
			<div id="menutab-keys">
				<table id="keys"></table>
			    <div id="keys_pager"></div> 
			</div>
			<div id="menutab-accesslog">
				<table id="accesslog"></table>
			    <div id="accesslog_pager"></div> 
			</div>
			<div id="menutab-readers">
				<table id="readers"></table>
			    <div id="readers_pager"></div> 
			</div>
		</div>
	</body>
</html>