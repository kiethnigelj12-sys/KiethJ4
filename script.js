import {
    initializeApp
} from "https://www.gstatic.com/firebasejs/12.7.0/firebase-app.js";

import {
    getDatabase,
    ref,
    onValue
} from "https://www.gstatic.com/firebasejs/12.7.0/firebase-database.js";


// =========================================================
// FIREBASE CONFIG
// =========================================================

const firebaseConfig = {

    apiKey:
        "AIzaSyAlvefyVlWc1CYCJupACS0OBI13u62E56Q",

    authDomain:
        "esp32-monitoring-de209.firebaseapp.com",

    databaseURL:
        "https://esp32-monitoring-de209-default-rtdb.europe-west1.firebasedatabase.app",

    projectId:
        "esp32-monitoring-de209",

    storageBucket:
        "esp32-monitoring-de209.firebasestorage.app",

    messagingSenderId:
        "1067358983711",

    appId:
        "1:1067358983711:web:b9ff36f1d5172efd24672a"

};


// =========================================================
// FIREBASE
// =========================================================

const app =
    initializeApp(
        firebaseConfig
    );

const db =
    getDatabase(app);


// =========================================================
// ELEMENTS
// =========================================================

const currentTemperature =
    document.getElementById(
        "currentTemperature"
    );

const currentHumidity =
    document.getElementById(
        "currentHumidity"
    );

const dateSelect =
    document.getElementById(
        "dateSelect"
    );

const readingsBody =
    document.getElementById(
        "readingsBody"
    );


// =========================================================
// DATA
// =========================================================

let allSensorData = {};

let sensorChart = null;


// =========================================================
// FORMAT TIME
// =========================================================

function formatTime(time) {

    if (!time) {

        return "--";

    }

    return time.replaceAll(
        "-",
        ":"
    );

}


// =========================================================
// SORT DATES
// =========================================================

function sortDates(dates) {

    return [...dates].sort(
        (a, b) =>
            new Date(b) -
            new Date(a)
    );

}


// =========================================================
// LATEST READING
// =========================================================

function getLatestReading(data) {

    const dates =
        Object.keys(
            data || {}
        );


    if (
        dates.length === 0
    ) {

        return null;

    }


    dates.sort(
        (a, b) =>
            new Date(b) -
            new Date(a)
    );


    const latestDate =
        dates[0];


    const times =
        Object.keys(
            data[latestDate] || {}
        );


    if (
        times.length === 0
    ) {

        return null;

    }


    times.sort().reverse();


    const latestTime =
        times[0];


    return {

        date:
            latestDate,

        time:
            latestTime,

        data:
            data[
                latestDate
            ][
                latestTime
            ]

    };

}


// =========================================================
// CURRENT VALUES
// =========================================================

function updateCurrentValues() {

    const latest =
        getLatestReading(
            allSensorData
        );


    if (!latest) {

        currentTemperature.textContent =
            "--";

        currentHumidity.textContent =
            "--";

        return;

    }


    const temperature =
        Number(
            latest.data.temperature
        );


    const humidity =
        Number(
            latest.data.humidity
        );


    currentTemperature.textContent =
        Number.isFinite(
            temperature
        )
            ? temperature.toFixed(1)
            : "--";


    currentHumidity.textContent =
        Number.isFinite(
            humidity
        )
            ? humidity.toFixed(1)
            : "--";

}


// =========================================================
// POPULATE ONE DATE SELECTOR
// =========================================================

function populateDates() {

    const dates =
        sortDates(
            Object.keys(
                allSensorData || {}
            )
        );


    dateSelect.innerHTML =
        "";


    if (
        dates.length === 0
    ) {

        const option =
            document.createElement(
                "option"
            );

        option.value = "";

        option.textContent =
            "No dates available";


        dateSelect.appendChild(
            option
        );

        return;

    }


    const defaultOption =
        document.createElement(
            "option"
        );

    defaultOption.value =
        "";

    defaultOption.textContent =
        "Select Date";


    dateSelect.appendChild(
        defaultOption
    );


    dates.forEach(
        date => {

            const option =
                document.createElement(
                    "option"
                );

            option.value =
                date;

            option.textContent =
                date;


            dateSelect.appendChild(
                option
            );

        }
    );


    // Automatically select latest date

    const latestDate =
        dates[0];


    dateSelect.value =
        latestDate;


    // Update table

    displayReadings(
        latestDate
    );


    // Update graph

    updateGraph(
        latestDate
    );

}


// =========================================================
// DISPLAY TABLE
// =========================================================

function displayReadings(date) {

    readingsBody.innerHTML =
        "";


    if (
        !date ||
        !allSensorData[date]
    ) {

        return;

    }


    const readings =
        allSensorData[date];


    const times =
        Object.keys(
            readings
        )
        .sort()
        .reverse();


    times.forEach(
        time => {

            const reading =
                readings[time] || {};


            const row =
                document.createElement(
                    "tr"
                );


            // DATE

            const dateCell =
                document.createElement(
                    "td"
                );

            dateCell.textContent =
                date;


            // TIME

            const timeCell =
                document.createElement(
                    "td"
                );

            timeCell.textContent =
                formatTime(
                    time
                );


            // TEMPERATURE

            const temperatureCell =
                document.createElement(
                    "td"
                );


            const temperature =
                Number(
                    reading.temperature
                );


            temperatureCell.textContent =
                Number.isFinite(
                    temperature
                )
                    ? temperature.toFixed(1) + " °C"
                    : "--";


            // HUMIDITY

            const humidityCell =
                document.createElement(
                    "td"
                );


            const humidity =
                Number(
                    reading.humidity
                );


            humidityCell.textContent =
                Number.isFinite(
                    humidity
                )
                    ? humidity.toFixed(1) + " %"
                    : "--";


            // ADD CELLS

            row.appendChild(
                dateCell
            );

            row.appendChild(
                timeCell
            );

            row.appendChild(
                temperatureCell
            );

            row.appendChild(
                humidityCell
            );


            readingsBody.appendChild(
                row
            );

        }
    );

}


// =========================================================
// GRAPH
// =========================================================

function updateGraph(date) {

    if (!date) {

        return;

    }


    if (
        !allSensorData[date]
    ) {

        return;

    }


    const readings =
        allSensorData[date];


    const times =
        Object.keys(
            readings
        ).sort();


    const labels = [];

    const temperatures = [];

    const humidities = [];


    times.forEach(
        time => {

            const reading =
                readings[time] || {};


            const temperature =
                Number(
                    reading.temperature
                );


            const humidity =
                Number(
                    reading.humidity
                );


            labels.push(
                formatTime(
                    time
                )
            );


            temperatures.push(
                Number.isFinite(
                    temperature
                )
                    ? temperature
                    : null
            );


            humidities.push(
                Number.isFinite(
                    humidity
                )
                    ? humidity
                    : null
            );

        }
    );


    const canvas =
        document.getElementById(
            "sensorChart"
        );


    if (!canvas) {

        return;

    }


    const ctx =
        canvas.getContext(
            "2d"
        );


    if (sensorChart) {

        sensorChart.destroy();

    }


    sensorChart =
        new Chart(
            ctx,
            {

                type:
                    "line",


                data:
                    {

                        labels:
                            labels,


                        datasets:
                            [

                                {

                                    label:
                                        "Temperature (°C)",

                                    data:
                                        temperatures,

                                    tension:
                                        0.35,

                                    borderWidth:
                                        3,

                                    pointRadius:
                                        3,

                                    yAxisID:
                                        "temperatureAxis"

                                },


                                {

                                    label:
                                        "Humidity (%)",

                                    data:
                                        humidities,

                                    tension:
                                        0.35,

                                    borderWidth:
                                        3,

                                    pointRadius:
                                        3,

                                    yAxisID:
                                        "humidityAxis"

                                }

                            ]

                    },


                options:
                    {

                        responsive:
                            true,

                        maintainAspectRatio:
                            false,


                        interaction:
                            {

                                mode:
                                    "index",

                                intersect:
                                    false

                            },


                        plugins:
                            {

                                legend:
                                    {

                                        labels:
                                            {

                                                color:
                                                    "#ffffff",

                                                font:
                                                    {

                                                        size:
                                                            13

                                                    }

                                            }

                                    },


                                tooltip:
                                    {

                                        callbacks:
                                            {

                                                label:
                                                    function (
                                                        context
                                                    ) {

                                                        return (
                                                            context.dataset.label +
                                                            ": " +
                                                            context.parsed.y
                                                        );

                                                    }

                                            }

                                    }

                            },


                        scales:
                            {

                                x:
                                    {

                                        ticks:
                                            {

                                                color:
                                                    "#e5caff"

                                            },

                                        grid:
                                            {

                                                color:
                                                    "rgba(255,255,255,0.08)"

                                            }

                                    },


                                temperatureAxis:
                                    {

                                        type:
                                            "linear",

                                        position:
                                            "left",

                                        title:
                                            {

                                                display:
                                                    true,

                                                text:
                                                    "Temperature °C",

                                                color:
                                                    "#e5caff"

                                            },

                                        ticks:
                                            {

                                                color:
                                                    "#e5caff"

                                            },

                                        grid:
                                            {

                                                color:
                                                    "rgba(255,255,255,0.08)"

                                            }

                                    },


                                humidityAxis:
                                    {

                                        type:
                                            "linear",

                                        position:
                                            "right",

                                        title:
                                            {

                                                display:
                                                    true,

                                                text:
                                                    "Humidity %",

                                                color:
                                                    "#e5caff"

                                            },

                                        ticks:
                                            {

                                                color:
                                                    "#e5caff"

                                            },

                                        grid:
                                            {

                                                drawOnChartArea:
                                                    false

                                            }

                                    }

                            }

                    }

            }
        );

}


// =========================================================
// ONE DATE SELECTOR CONTROLS BOTH TABLE AND GRAPH
// =========================================================

dateSelect.addEventListener(
    "change",
    () => {

        const selectedDate =
            dateSelect.value;


        // Update table

        displayReadings(
            selectedDate
        );


        // Update graph

        updateGraph(
            selectedDate
        );

    }
);


// =========================================================
// FIREBASE REALTIME DATA
// =========================================================

const sensorRef =
    ref(
        db,
        "ESP32_Data"
    );


onValue(
    sensorRef,

    snapshot => {

        allSensorData =
            snapshot.val() || {};


        updateCurrentValues();

        populateDates();

    },

    error => {

        console.error(
            "Firebase error:",
            error
        );

    }
);