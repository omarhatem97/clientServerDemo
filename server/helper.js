prepareResponse = (requestObject, res) => {
  let op = requestObject["op"];
  let num = requestObject["num"];
  let num1 = requestObject["num1"];

  switch (op) {
    case "diff":
      res.write(
        "data: " + `Diff operation occured and result is ${num - num1} ----\n\n`
      );
      break;
    case "add":
      res.write(
        "data: " + `Diff operation occured and result is ${num - num1} ----\n\n`
      );
      break;
    default:
      break;
  }
};
