function l(o) {
	console.log(o);
}

function resizeTab(id) {
	gridParentWidth = $('#gbox_' + id).parent().width();
	$('#' + id).jqGrid('setGridWidth',gridParentWidth);
}

function resizeAllTabs() {
	grids = ['accesslog', 'keys', 'members', 'readers'];
	
	for (var i=0; i<grids.length; i++) {
		resizeTab(grids[i]);
	}
}

function loadKeysTab() {
	$('#keys').jqGrid({
		url: 'ajax.php?a=keys',
		editurl: 'ajax.php?a=editkey',
		datatype: 'json',
		colNames: ['ID', 'Owner Name', 'Hash', 'Key Type', 'Key Active?', 'Last Edit', 'Added On', ],
		colModel: [
			{name: 'id', index: 'id', editable: false, width: 30, align: 'center'},
			{name: 'ownerid', index: 'ownerid', editable: true, formatter: 'select', edittype: 'select', editoptions: { value: "0:Not assigned" }, align: 'center'},
			{name: 'hash', index: 'hash', width: 200, editable: false, align: 'center'},
			{name: 'ktype', index: 'ktype', width: 50, editable: true, formatter: 'select', edittype: 'select', editoptions: {value: 'master:Master;member:Member;guest:Guest;unknown:Unknown;lost:Lost'}, width: 50, align: 'center'},
			{name: 'active', index: 'active', editable: true, formatter: 'select', edittype: 'select', editoptions: {value: '1:Active;0:Not Active'}, width: 50, align: 'center'},
			{name: 'ts_edited', index: 'ts_edited', width: 100, align: 'center'},
			{name: 'ts_created', index: 'ts_created', align: 'center'}
		],
		caption: "Keys",
		height: '100%',
		rowNum: 15,
	    emptyrecords: "Nothing to see here.",
	   	pager: "#keys_pager",
	    viewrecords: true,
	   	sortname: 'id',
	   	sortorder: 'asc',
		autowidth: true,
		width: '100%',
		loadonce: false,
		scroll: false,
		gridComplete: resizeTab('keys')
	});
	
	$('#keys').jqGrid('navGrid', '#keys_pager', {edit: true, del: false, add: false, search: true});
}

function loadAccessLogTab() {
	$('#accesslog').jqGrid({
		url: 'ajax.php?a=accesslog',
		datatype: 'json',
		colNames: ['ID', 'Access time', 'Owner Name', 'Hash', 'Key Type', 'Key Active?', 'Area Request', 'Access Granted?'],
		colModel: [
			{name: 'id', index: 'id', editable: false, width: 30, align: 'center'},
			{name: 'ts_created', index: 'ts_created', align: 'center'},
			{name: 'ownername', index: 'ownername', align: 'center'},
			{name: 'hash', index: 'hash', width: 200, align: 'center'},
			{name: 'ktype', index: 'ktype', width: 50, editable: false, formatter: 'select', edittype: 'select', editoptions: {value: 'master:Master;member:Member;guest:Guest;unknown:Unknown;lost:Lost'}, align: 'center'},
			{name: 'active', index: 'active', editable: false, formatter:'select', edittype:"select", editoptions: {value:"1:Active;0:Not Active"}, width: 100, align: 'center'},
			{name: 'request', index: 'request', width: 100, align: 'center'},
			{name: 'agranted', index: 'agranted',editable: false, formatter:'select', edittype:"select", editoptions: {value:"1:Yes;0:No"},  width: 100, align: 'center'}
		],
		caption: "Access Log",
		height: '100%',
		rowNum: 15,
	    emptyrecords: "Nothing to see here.",
	   	pager: "#accesslog_pager",
	    viewrecords: true,
	   	sortname: 'ts_created',
	   	sortorder: 'desc',
		autowidth: true,
		width: '100%',
		loadonce: false,
		scroll: false,
		gridComplete: resizeTab('accesslog')
	});
	
	$('#accesslog').jqGrid('navGrid', '#accesslog_pager', {edit: false, del: false, add: false, search: false});
}

/*
CREATE TABLE `readers` (
	id INTEGER NOT NULL AUTO_INCREMENT,
	readername VARCHAR(32) UNIQUE NOT NULL,
 	ts_lastseen timestamp DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
 	ts_created timestamp DEFAULT CURRENT_TIMESTAMP,
 	PRIMARY KEY (id)
) ENGINE=InnoDB;
*/

function loadReadersTab() {
	$('#readers').jqGrid({
		url: 'ajax.php?a=readers',
		datatype: 'json',
		colNames: ['ID', 'Reader Name', 'Last seen', 'First seen'],
		colModel: [
			{name: 'id', index: 'id', editable: false, width: 30, align: 'center'},
			{name: 'readername', index: 'readername', editable: false, width: 100, align: 'center'},
			{name: 'ts_lastseen', index: 'ts_lastseen', editable: false, align: 'center'},
			{name: 'ts_created', index: 'ts_created', editable: false, align: 'center'},
		],
		caption: "RFID Readers",
		height: '100%',
		rowNum: 15,
	    emptyrecords: "Nothing to see here.",
	   	pager: "#readername",
	    viewrecords: true,
	   	sortname: 'ts_created',
	   	sortorder: 'desc',
		autowidth: true,
		width: '100%',
		loadonce: false,
		scroll: false,
		gridComplete: resizeTab('readers')
	});
	
	$('#readers').jqGrid('navGrid', '#readers_pager', {edit: false, del: false, add: false, search: false});
}

function loadMembersTab() {
	$('#members').jqGrid({
		url: 'ajax.php?a=members',
		editurl: 'ajax.php?a=editmember',
		datatype: 'json',
		colNames: ['ID', 'Member ID', 'Name', 'Email', 'Phonenumber', 'Active', 'Last Edit', 'Added On'],
		colModel: [
			{name: 'id', index: 'id', editable: false, width: 30, align: 'center'},
			{name: 'mid', index: 'mid', editable: true, width: 50, align: 'center'},
			{name: 'mname', index: 'mname', editable:true, width: 150, align: 'center'},
			{name: 'email', index: 'email', editable:true, width: 150, align: 'center'},
			{name: 'phonenumber', index: 'phonenumber', editable: true, width: 100, align: 'center'},
			{name: 'active', index: 'active', editable: true, formatter:'select', edittype:"select", editoptions: {value:"1:Active;0:Not Active"}, width: 50, align: 'center'},
			{name: 'ts_edited', index: 'ts_edited', width: 100, align: 'center'},
			{name: 'ts_created', index: 'ts_created', width: 100, align: 'center'}
		],
		caption: "Members",
		height: '100%', 
		rowNum: 10,
	    emptyrecords: "Nothing to see here.",
	   	pager: "#members_pager",
	    viewrecords: true,
	   	sortname: 'ts_edited, ts_created',
	   	sortorder: 'desc',
		autowidth: true,
		width: '100%',
		loadonce: false,
		scroll: false,
		gridComplete: function() {
			loadAllMembers();
			resizeTab('members');
		}
	});
	
	$('#members').jqGrid('navGrid', '#members_pager', {edit: true, del: false, add: true, search: false});
}

function loadAllMembers() {
	$.ajax({
		url: 'ajax.php?a=allmembers',
		method: 'POST',
		success: function(res) {
			updateKeysTabMemberSelection(res['rows']);
		}
	});
}

function updateKeysTabMemberSelection(members) {
	l(members);
	var val = '';
	
	for (var i=0; i<members.length; i++) {
		val += members[i].id + ':' + members[i].mname + ';';
	}
	
	$('#keys').jqGrid('setColProp', 'ownerid', { editoptions: { value: val } });
}

function initialize() {
	$('#menutabs').tabs({
		activate: function(evt, ui) {
			
			if (ui.newTab.index() != 0) {
				var r = new RegExp(/.*\-([a-z]*)/);
				var m = ui.newPanel.selector.match(r);
				$('#' + m[1]).trigger('reloadGrid');
				resizeTab(m[1]);
			}
		}
	});
	
	loadAccessLogTab();
	loadKeysTab();
	loadMembersTab();
	loadAllMembers();
	loadReadersTab();
	
	resizeAllTabs();

	$(window).resize(function() {
		resizeAllTabs();
	});
}