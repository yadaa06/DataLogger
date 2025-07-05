let myChart;
const ctx = document.getElementById('sensorChart').getContext('2d');
const loader = document.getElementById('loader');
const readNowBtn = document.getElementById('readNowButton');

async function initializeChart() {
    try {
        const response = await fetch('/dht_history');
        const data = await response.json();

        const labels = data.history.map(d => new Date(d.timestamp * 1000).toLocaleTimeString());
        const tempData = data.history.map(d => d.temperature);
        const humidityData = data.history.map(d => d.humidity);

        myChart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: labels,
                datasets: [{
                    label: 'Temperature (°F)',
                    data: tempData,
                    borderColor: 'rgba(255, 99, 132, 1)',
                    yAxisID: 'y'
                }, {
                    label: 'Humidity (%)',
                    data: humidityData,
                    borderColor: 'rgba(54, 162, 235, 1)',
                    yAxisID: 'y1'
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                scales: {
                    x: { title: { display: true, text: 'Time' } },
                    y: {
                        type: 'linear',
                        display: true,
                        position: 'left',
                        title: { display: true, text: 'Temperature (°F)' }
                    },
                    y1: {
                        type: 'linear',
                        display: true,
                        position: 'right',
                        title: { display: true, text: 'Humidity (%)' },
                        grid: { drawOnChartArea: false }
                    }
                }
            }
        });
    } catch (error) {
        console.error("Error initializing chart:", error);
    }
}

async function updateDHTdata() {
    loader.classList.remove('hidden');
    loader.classList.add('visible');
    readNowBtn.disabled = true;
    try {
        const response = await fetch('/dht_data');
        const data = await response.json();

        document.getElementById('temperature').textContent = data.temperature.toFixed(2);
        document.getElementById('humidity').textContent = data.humidity.toFixed(1);

        const now = new Date();
        const newLabel = now.toLocaleTimeString();
        document.getElementById('lastupdated').textContent = newLabel;

        console.log("Data updated successfully:", data);

        if (myChart) {
            myChart.data.labels.push(newLabel);
            myChart.data.datasets[0].data.push(data.temperature);
            myChart.data.datasets[1].data.push(data.humidity);

            if (myChart.data.labels.length > 60) {
                myChart.data.labels.shift();
                myChart.data.datasets.forEach(dataset => dataset.data.shift());
            }

            myChart.update();
        }

    } catch (error) {
        console.error("Error fetching DHT data:", error);
        document.getElementById('temperature').textContent = "Error";
        document.getElementById('humidity').textContent = "Error";
        document.getElementById('lastupdated').textContent = "Error";
    } finally {
        loader.classList.remove('visible');
        loader.classList.add('hidden');
        readNowBtn.disabled = false;
    }
}

if (readNowBtn) {
    readNowBtn.addEventListener('click', () => {
        console.log("Read Now Button Clicked");
        updateDHTdata();
    });
}

document.addEventListener('DOMContentLoaded', () => {
    initializeChart();
    updateDHTdata();
});
setInterval(updateDHTdata, 60000);