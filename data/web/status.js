// ************************************************
window.addEventListener('DOMContentLoaded', init, false);
function init() {
  GetInitData();
}

// ************************************************
function GetInitData() {
  var data = {};
  data['action'] = "GetInitData";
  data['subaction'] = "status";
  requestData(JSON.stringify(data), false, MyCallback);
}

// ************************************************
function MyCallback() {
  document.querySelector("#loader").style.visibility = "hidden";
  document.querySelector("body").style.visibility = "visible";
}

// ************************************************
