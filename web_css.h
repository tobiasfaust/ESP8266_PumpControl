const char STYLE_CSS[] PROGMEM = R"=====(
  body {
   font-size:140%;
   font-family: Verdana,Arial,Helvetica,sans-serif;
 } 
 h2 {
   color: #2e6c80;
   text-align: center;
 } 
 td, th {
     font-size: 14px;
 }
 .button { 
     padding: 4px 16px; 
     margin: 4px;
     background-color: #07D;
     color: #FFF;
     text-decoration: none;
     border-radius: 4px;
     border: none;
 }
.button:hover {background: #369;}
 input, select, textarea {
     margin: 4px; 
     padding: 4px 8px; 
     border-radius: 4px; 
     background-color: #eee; 
     border-style: solid; 
     border-width: 1px; 
     border-color: gray;
 }
 input:hover {background-color: #ccc; }
 select:hover {background-color: #ccc; }
 textarea:hover {background-color: #ccc; }
 .editorDemoTable {
     border-spacing: 0;
     background-color: #FFF8C9;
     margin-left: auto; margin-right: auto;
 } 
 .editorDemoTable thead {
     color: #000000;
     background-color: #2E6C80;
 }
 .editorDemoTable thead td {
     font-weight: bold;
     font-size: 13px;
 }
 .editorDemoTable td {
     border: 1px solid #777;
     margin: 0 !important;
     padding: 2px 3px;
 }
 .navi {
   border-bottom: 3px solid #777;
   padding: 5px;
   text-align: center;
 }
 .navi_active {
   background-color: #CCCCCC;
   border: 3px solid #777;;
   border-bottom: none;
 }
)=====";
