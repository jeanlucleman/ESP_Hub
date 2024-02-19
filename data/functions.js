/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
// Méthode 1 pour récupérer des champs du formulaires dans le cpp
$(document).ready(function(){
    $("#id_btn_validation1").click(function(){
      var ssid = $("#selectedSSID").val();
      var psw = $("#id_password").val();
      $.post("setCredentials",{
			  valeurSSID: ssid,
			  valeurPSW: psw
        });
    });
});

// Méthode 2 pour récupérer des champs du formulaires dans le cpp
// Cette fonction est activée par l'event click sur le bouton 2 du formulaire
function setNewCredentials(){
	var ssid = document.getElementById("selectedSSID").value;
  var psw = document.getElementById("id_password").value;
  var espName = document.getElementById("id_name").value;
	var xhttp = new XMLHttpRequest();
  xhttp.open("POST", "setCredentials", true);
  var params = "valeurSSID=" + ssid + "&valeurPSW=" + psw + "&valeurName=" + espName;
  alert(params);
	//Send the proper header information along with the request
	xhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
	xhttp.setRequestHeader("Content-length", params.length);
	xhttp.setRequestHeader("Connection", "close");
	xhttp.onreadystatechange = function() {//Call a function when the state changes.
		if(xhttp.readyState == 4 && xhttp.status == 200) {
			alert(xhttp.responseText);
		}
	}
	xhttp.send(params);
}




// Cette fonction est activée par l'event click sur le bouton Save change du formulaire Domus
function updateDomusSettings(){
	var ucIp = document.getElementById("idUcIp").value;
  var ucPort = document.getElementById("idUcPort").value;
  var wifiLedPinNumber = document.getElementById("idWifiLedPinNumber").value;
  var retourLedPirPinNumber = document.getElementById("idRetourLedPirPinNumber").value;
  var redLedRadarPinNumber = document.getElementById("idRedLedRadarPinNumber").value;
  var dht11PinNumber = document.getElementById("idDht11PinNumber").value;
  var relayPinNumber = document.getElementById("idRelayPinNumber").value;
  var ldrPinNumber = document.getElementById("idLdrPinNumber").value;
  var pushButtonPinNumber = document.getElementById("idPushButtonPinNumber").value;
  var radarPinNumber = document.getElementById("idradarPinNumber").value;
  var radarNewDetectionTime = document.getElementById("idRadarNewDetectionTime").value;
  var pirPinNumber = document.getElementById("idpirPinNumber").value;
  var pirNewDetectionTime  = document.getElementById("idPirNewDetectionTime").value;
	var xhttp = new XMLHttpRequest();
	xhttp.open("POST", "updateDomusSettings", true);
  var params = "ucIp=" + ucIp 
  + "&ucPort=" +  ucPort
  + "&wifiLedPinNumber=" +  wifiLedPinNumber
  + "&retourLedPirPinNumber=" +  retourLedPirPinNumber
  + "&redLedRadarPinNumber=" +  redLedRadarPinNumber
  + "&dht11PinNumber=" + dht11PinNumber 
  + "&relayPinNumber=" +  relayPinNumber
  + "&ldrPinNumber=" +  ldrPinNumber
  + "&pushButtonPinNumber=" +  pushButtonPinNumber
  + "&radarPinNumber=" +  radarPinNumber
  + "&radarNewDetectionTime=" +  radarNewDetectionTime
  + "&pirPinNumber=" +  pirPinNumber
  + "&pirNewDetectionTime=" +  pirNewDetectionTime
  ;
  alert(params);
	//Send the proper header information along with the request
	xhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
	xhttp.setRequestHeader("Content-length", params.length);
	xhttp.setRequestHeader("Connection", "close");
	xhttp.onreadystatechange = function() {//Call a function when the state changes.
		if(xhttp.readyState == 4 && xhttp.status == 200) {
			alert(xhttp.responseText);
		}
	}
	xhttp.send(params);
}

// cette fonction est activée par le script de la page HTML. Elle récupère les données des réseaux wifi
//  qui ont été placées dans le champ caché listNetworks et qui contient le placeholder.
// Ce place holder a été rempli par le serveur lorsqu'il reçoit une requete à la racine (192.168.4.1 par défaut)
// Voir :  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
//      getListNetwork();
//      request->send(SPIFFS, "/index.html", String(), false, processor);
//     });
function fillNetworks()
  {
    var table=document.getElementById('tableNetworks');
    var myarray=document.getElementById('listNetworks').value;
    var netData=myarray.split("/");
    for (var i=0;i<netData.length;i++){
      var x=netData[i].split(",");
      var newRow=document.createElement("tr");
      table.appendChild(newRow);
      newRow.addEventListener("click", function() {selectNetwork(this.name);});
      newRow.name=x[0];
      var dataSSID=document.createElement("td");
      dataSSID.textContent=x[0];
      newRow.appendChild(dataSSID);
      var dataLevel=document.createElement("td");
      dataLevel.textContent=x[1];
      newRow.appendChild(dataLevel);
      var dataSecurity=document.createElement("td");
      dataSecurity.textContent=x[2];
      newRow.appendChild(dataSecurity);
    }
  }

function selectNetwork(data)
  {
    document.getElementById("id_password").disabled=false;
    document.getElementById("selectedSSID").value=data;
  }

function pressEnter(event) 
  {
    var code=event.which || event.keyCode; //Selon le navigateur c'est which ou keyCode
    if (code==13) { //le code de la touche Enter
      document.getElementById("id_btn_validation1").disabled=false;
      document.getElementById("id_btn_validation2").disabled=false;
    }
  }

function onButton() 
  {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "on", true);
    xhttp.send();
  }

function reboot(){
  var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "reboot", true);
  xhttp.send();

}

// setInterval(function getData()
// {
//     var xhttp = new XMLHttpRequest();
//     var x=0;
//     xhttp.onreadystatechange = function()
//     {
//         if(this.readyState == 4 && this.status == 200)
//         {
//           var res = this.responseText.split("$$");
//           for (i = 0; i < res.length; i++) {
//           var node = document.createElement("li");                 // Create a <li> node
//           node.className="logLine";
//           var textnode = document.createTextNode(res[i]);         // Create a text node
//           node.appendChild(textnode);
          
//           document.getElementById("idLogBox").appendChild(node); 
//           }
//           // var node = document.createElement("LI");                 // Create a <li> node
//           // var textnode = document.createTextNode("Water");         // Create a text node
//           // node.appendChild(textnode);                              // Append the text to <li>
//           // document.getElementById("idLogBox").appendChild(node); 
//           //document.getElementsByName("valeurLuminosite").innerHTML = this.responseText;
//         }
//     };

//     xhttp.open("GET", "getNewLine", true);
//     xhttp.send();
// }, 1000);