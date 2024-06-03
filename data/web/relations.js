// ************************************************
window.addEventListener('DOMContentLoaded', init, false);
function init() {
  GetInitData();
}

// ************************************************
function GetInitData() {
  var data = {};
  data.action = "GetInitData";
  data.subaction = "relations";
  requestData(JSON.stringify(data), false, MyCallback);
}

// ************************************************
function MyCallback() {
  CreateSelectionListFromInputField('input[type=number][id$=ConfiguredPort]', [configuredPorts]);
  validate_identifiers("maintable");
}

// ************************************************
