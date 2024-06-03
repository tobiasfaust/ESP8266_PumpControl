// ************************************************
window.addEventListener('DOMContentLoaded', init, false);
function init() {
  GetInitData();
}

// ************************************************
function GetInitData() {
  var data = {};
  data.action = "GetInitData";
  data.subaction = "sensorconfig";
  requestData(JSON.stringify(data), false, MyCallback);
}

// ************************************************
function MyCallback() {
  CreateSelectionListFromInputField('input[type=number][id^=GpioPin]', [gpio]);
  CreateSelectionListFromInputField('input[type=number][id^=AnalogPin]', [gpioanalog]);
}

// ************************************************
