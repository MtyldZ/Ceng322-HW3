/*
* UMUT YILDIZ 260201028
* A node application to find average values of many tests
* In order to use first compile main.c to a.out
* -> execute `bash run.sh`
* -> execute `node index.js` (you may need node installed - to install, exec `sudo apt install nodejs`)
* -> Done!
* output results are in thread order: 1st = 1, 2nd = 2, 3rd = 4, 4th = 8, 5th = 16
*
* */

const fs = require('fs');
const files = [
    "out-1000.csv",
    "out-10000.csv",
    "out-100000.csv",
]
files.forEach(file => {
    fs.readFile(file, 'utf8', (err, data) => {
        if (err) {
            console.error(err);
            return;
        }

        // print the file name
        console.log(file);
        const lines = data.split('\n'); // get line count
        const repetitionCount = lines.length / 5; // get repetition amount
        // repetition count: trial count of process with same arguments
        const averages = {};

        lines.forEach((line) => {
            if (line !== '') {
                const [thread, _, timeSpent] = line.split(",");
                if (averages[thread]) {
                    averages[thread] += parseFloat(timeSpent);
                } else {
                    averages[thread] = parseFloat(timeSpent);
                }
            }
        });
        // print the averages
        Object.keys(averages).forEach(key => {
            averages[key] /= repetitionCount; // calculate average
            // print the average result in format that excel excepts directly
            // the floating point uses comma [ wrong: 1.15 correct: 1,15 ]
            console.log(averages[key].toFixed(7).replace(".", ","));
        })
    });
})

