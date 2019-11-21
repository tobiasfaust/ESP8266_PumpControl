const char JAVASCRIPT[] PROGMEM = R"=====(
const gpio = [16,5,4,0,2,14,12,13,15,1,3]; // alle GPIOs des ESP8266
const gpioname = ['D0','D1','D2/SDA', 'D3/SCL', 'D4','D5','D6', 'D7','D8','RX','TX'];

//document.getElementById('gpio1').addEventListener('focus', Txt2Select, false);
//window.onload = function () { function init(); }
window.addEventListener('load', init, false);

function init() {
  SetConfiguredPorts();
  SetAvailablePorts();
}
function SetConfiguredPorts() {
  var _parent, _select, _option, i, j, k;
  var objects = document.querySelectorAll('[id^=ConfiguredPorts]');
  for( j=0; j< objects.length; j++) {
    _parent = objects[j].parentNode;
    _select = document.createElement('select');
    _select.id = objects[j].id;
    _select.name = objects[j].name;
    for ( i = 0; i < configuredPorts.length; i += 1 ) {
        _option = document.createElement( 'option' );
        _option.value = configuredPorts[i]['port']; 
        _option.text  = configuredPorts[i]['name'];
        if(objects[j].value == configuredPorts[i]['port']) { _option.selected = true;}
        _select.add( _option ); 
    }
    _parent.removeChild( objects[j] );
    _parent.appendChild( _select );
  }
}

function SetAvailablePorts() {
  var _parent, _select, _option, i, j, k;
  var objects = document.querySelectorAll('[id^=AllePorts], [id^=GpioPorts], [id^=GpioPin]');
  for( j=0; j< objects.length; j++) {
    _parent = objects[j].parentNode;
    _select = document.createElement('select');
    _select.id = objects[j].id;
    _select.name = objects[j].name;
    for ( i = 0; i < gpio.length; i += 1 ) {
        _option = document.createElement( 'option' );
        if (objects[j].id.match(/^GpioPin.*/)) {
          _option.value = gpio[i]; 
          if(objects[j].value == (gpio[i])) { _option.selected = true;}
        } else {
          _option.value = gpio[i]+200; 
          if(gpio_disabled.indexOf(gpio[i])>=0) {_option.disabled = true;}
          if(objects[j].value == (gpio[i]+200)) { _option.selected = true;}
        }
        _option.text  = gpioname[i];
        _select.add( _option ); 
    }
    if (objects[j].id.match(/^Alle.*/)) {
      for ( k = 0; k < pcf_found.length; k++ ) {
        for ( i = 0; i < 8; i += 1 ) {
            _option = document.createElement( 'option' );
            _option.value = _option.text = pcf_found[k]+i;
            if(objects[j].value == pcf_found[k]+i) { _option.selected = true;}
            _select.add( _option );
        }
      }
    }
    _parent.removeChild( objects[j] );
    _parent.appendChild( _select );
  }
}
)=====";
