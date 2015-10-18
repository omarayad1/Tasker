var data = [];
var totalPoints = 5;
var updateInterval = 2000;
var now = new Date().getTime();
 
function GetData() {
    data.shift();
    var loc = $('#my-data').data().curr;
    var requests = function(){
        if (data.length < totalPoints) {
            $.get('/cpu',function(shit){    
                    var y = shit.data[loc].usage;
                    console.log(loc);
                    var temp = [now += updateInterval, y]; //data format [x, y]
                    data.push(temp);
                    setTimeout(requests,100);
                }
            );
        }
    }
    requests();
}


var options ={
series: {
    lines: {
        show: true,
        lineWidth: 1.2,
        fill: true
    }
},

xaxis: {
    mode: "time",
    tickSize: [2, "second"],
    tickFormatter: function (v, axis) {
        var date = new Date(v);
 
        if (date.getSeconds() % 20 == 0) {
            var hours = date.getHours() < 10 ? "0" + date.getHours() : date.getHours();
            var minutes = date.getMinutes() < 10 ? "0" + date.getMinutes() : date.getMinutes();
            var seconds = date.getSeconds() < 10 ? "0" + date.getSeconds() : date.getSeconds();
 
            return hours + ":" + minutes + ":" + seconds;
        } else {
            return "";
        }
    },
    axisLabel: "Time",
    axisLabelUseCanvas: true,
    axisLabelFontSizePixels: 12,
    axisLabelFontFamily: 'Verdana, Arial',
    axisLabelPadding: 10
},
yaxis: {
    min: 0,
    max: 100,                          
    tickFormatter: function (v, axis) {
        if (v % 10 == 0) {
            return v + "%";
        } else {
            return "";
        }
    },
    axisLabel: "CPU loading",
    axisLabelUseCanvas: true,
    axisLabelFontSizePixels: 12,
    axisLabelFontFamily: 'Verdana, Arial',
    axisLabelPadding: 6
},
    legend: {        
        labelBoxBorderColor: "#fff"
    },
    grid: {                
        backgroundColor: "#000000",
        tickColor: "#008040"
    }
}
$(document).ready(function () {
    GetData();
 
    dataset = [
        { label: "CPU", data: data, color: "#00FF00" }
    ];
 
    $.plot($("#flot-placeholder1"), dataset, options);
 
    function update() {
        GetData();
 
        $.plot($("#flot-placeholder1"), dataset, options)
        setTimeout(update, updateInterval);
    }
 
    update();
});