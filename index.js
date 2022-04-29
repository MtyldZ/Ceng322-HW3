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
const size = 10;
files.forEach(file => {
    fs.readFile(file, 'utf8', (err, data) => {
        if (err) {
            console.error(err);
            return;
        }
        console.log(file);
        const averages = {};
        data.split("\n").forEach((line) => {
            if (line !== '') {
                const [thread, _, timeSpent] = line.split(",");
                if (averages[thread]) {
                    averages[thread] += parseFloat(timeSpent);
                } else {
                    averages[thread] = parseFloat(timeSpent);
                }
            }
        });
        Object.keys(averages).forEach(key => {
            averages[key] /= size;
            // print the average result in format that excel excepts directly
            // the floating point is comma [ wrong: 1.15 correct: 1,15 ]
            console.log(averages[key].toFixed(7).replace(".", ","));
        })
    });
})

