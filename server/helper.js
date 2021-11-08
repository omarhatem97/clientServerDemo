//Helper functions-----------------------------
exports.prepareResponse = function (requestObject, res) {
  return new Promise((resolve, rej) => {
    let op = requestObject["op"];
    let num = requestObject["num"];
    let num1 = requestObject["num1"];
    console.log("requestObject", requestObject);
    let response = {};

    switch (op) {
      case "diff":
        subtract(num, num1).then((ans) => {
          resolve(
            (response = {
              op: "subtract",
              result: ans,
            })
          );
        });

        break;
      case "add":
        add(num, num1).then((ans) => {
          resolve(
            (response = {
              op: "add",
              result: ans,
            })
          );
        });
        break;
      default:
        resolve({});
        break;
    }
  });
};

//add
function add(a, b) {
  return new Promise((resolve, reject) => {
    let t = Math.floor(Math.random() * 20) * 1000;
    console.log("time: ", t);
    setTimeout(() => {
      let res = parseInt(a) + parseInt(b);
      resolve(res);
    }, t);
  });
}

function subtract(a, b) {
  return new Promise((resolve, reject) => {
    let t = Math.floor(Math.random() * 20) * 1000;
    console.log("time it will take: ", t / 1000);
    setTimeout(() => {
      let res = parseInt(a) - parseInt(b);
      resolve(res);
    }, t);
  });
}
