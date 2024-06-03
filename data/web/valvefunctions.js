/*******************************
copy first row of table and add it as clone 
*******************************/
function addrow(tableID) { 
  _table = document.getElementById(tableID);
  _table.rows[1].style.display = '';
  new_row = _table.rows[1].cloneNode(true);
  _table.appendChild(new_row);
  validate_identifiers(tableID);
}

/*******************************
delete a row in table
*******************************/
function delrow(object) { 
  var table = GetParentObject(object, 'TABLE');
  var rowIndex = GetParentObject(object, 'TR').rowIndex;
  if (table.rows.length > 2) {
    // erste Zeile ist das Template + Header, darf nicht entfernt werden
    table.deleteRow(rowIndex)
    validate_identifiers(table.id);
  }
}

/*******************************
recalculate all id´s, name´s
*******************************/
function validate_identifiers(tableID) {
  table = document.getElementById(tableID); 
  for( i=1; i< table.rows.length; i++) { 
    row = table.rows[i];
    row.cells[0].innerHTML = i; 
    objects = row.querySelectorAll('label, input, select, div, td');
    for( j=0; j< objects.length; j++) {
      if (objects[j].name) {objects[j].name = objects[j].name.replace(/(\d+)/, i-1);}
      if (objects[j].id) {objects[j].id = objects[j].id.replace(/(\d+)/, i-1);}
      if (objects[j].htmlFor) {objects[j].htmlFor = objects[j].htmlFor.replace(/(\d+)/, i-1);}
    }
  }
}

/*******************************
return the first parent object of tagName, e.g. TR
*******************************/
function GetParentObject(object, TargetTagName) {
  if (object.tagName == TargetTagName) {return object;}
  else if (object.parentNode === null) { return false;}
  else { return GetParentObject(object.parentNode, TargetTagName); }
}

/*******************************
return the port-value of selected row, 
object: anyone object of that row 
*******************************/
function GetPortOfRow(object) {
  var port = 0;
  var row = GetParentObject(object, 'TR')
  var objects = document.querySelectorAll('select[id*=AllePorts][name=port_a]');

  for( var i=0; i< objects.length; i++) {
    if(isVisible(objects[i]) && row == GetParentObject(objects[i], 'TR')) {
      port = objects[i].value;
    }
  }
  
  return port;
}

/************************************************
the "active" checkbox has pressed
*************************************************/
function ChangeEnabled(object) {
  var data = {};

  data['action'] = "EnableValve"
  data['newState'] = object.checked;
  data['port'] = GetPortOfRow(object);

  requestData(JSON.stringify(data));
}

/************************************************
the valbve type has changed
*************************************************/
function ChangeValve(object) {
  btn = document.getElementById(object.id);
  var data = {};

  data['action'] = "SetValve";
  data['subaction'] = object.id;
  data['newState'] = btn.value.replace(/^Set\ (.*)/, "$1");
  data['port'] = GetPortOfRow(object);
  
  requestData(JSON.stringify(data), false);
}

/************************************************
the "active" checkbox was press
*************************************************/
function ChangeType(object) {
  var _obj_n, _obj_b; 
  var row = GetParentObject(object, 'TR')
  val = object.value;

  var objects = document.querySelectorAll('div[id*=typ_]');

  for( var i=0; i< objects.length; i++) {
    if(row == GetParentObject(objects[i], 'TR')) {
      if (objects[i].id.match(/typ_n/)) {_obj_n = objects[i];}
      if (objects[i].id.match(/typ_b/)) {_obj_b = objects[i];}
    }
  }
 
  if (val == 'b') {
    // Typ "Bistabil"
    _obj_n.classList = "hide";
    _obj_b.classList = "";
  } else if (val == 'n'){
    // Typ "normal"
    _obj_n.classList = "";
    _obj_b.classList = "hide";
  } 
}
