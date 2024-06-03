// ************************************************
window.addEventListener('DOMContentLoaded', init, false);
function init() {
  GetInitData();
}

// ************************************************
function GetInitData() {
  var data = {};
  data.action = "GetInitData";
  data.subaction = "baseconfig";
  requestData(JSON.stringify(data), false, MyCallback);
}

// ************************************************
function MyCallback() {
  CreateSelectionListFromInputField('input[type=number][id^=GpioPin]', [gpio]);
  CreateSelectionListFromInputField('input[type=number][id*=ConfiguredPort]', [configuredPorts]);
  SetUpdateURL()
  FetchReleaseInfo();
}

// ************************************************

function SetUpdateURL() {
  var u = document.getElementById("au_url");
  u.value = update_url;
}

function FetchReleaseInfo() {
  fetch( update_url, {})
      .then (response => response.json())
      .then (json =>  {
        ProcessReleaseInfo(json)
      })
}

function ProcessReleaseInfo(json) {
	var _parent, _select, _optgroup_dev, _optgroup_pre, _optgroup_prd, _option;
  
  _select = document.getElementById('releases');
  _select.replaceChildren();
  
  _optgroup_dev = document.createElement('optgroup');
  _optgroup_pre = document.createElement('optgroup');
  _optgroup_prd = document.createElement('optgroup');
 
  _optgroup_dev.label = "Development";
  _optgroup_pre.label = "Prelive";
  _optgroup_prd.label = "Produktiv";
  
  if (Array.isArray(json)) {
    for (var i=0; i < json.length; i++) {
    	//console.log(json[i])
      _option = document.createElement('option');
      _option.value = json[i]['download-url'];
      _option.text = json[i].name + " (" + json[i].subversion + ")";
      //_option.selected
      if (json[i].stage == 'DEV')  { _optgroup_dev.appendChild(_option); }
      if (json[i].stage == 'PRE')  { _optgroup_pre.appendChild(_option); }
      if (json[i].stage == 'PROD') { _optgroup_prd.appendChild(_option); }
    } 
  }
  
  _select.add( _optgroup_prd );
  _select.add( _optgroup_pre );
  _select.add( _optgroup_dev ); 
}

function FetchRelease() {
  r = document.getElementById('release')
  console.log(r.value)
  fetch( r.value, {
  	responseType: 'blob'
  		})
      .then (response =>  response.blob())
      .then (blob => InstallRelease(blob))
      ;
}

function InstallRelease(BinaryBlob) {
  const formData = new FormData();
  formData.append("firmware", BinaryBlob, "firmware.bin");
    
  fetch('/update', {
      method: 'POST',
      body: formData,
    })
}

