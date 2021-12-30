# References
- [date/date.h](https://stackoverflow.com/questions/21021388/how-to-parse-a-date-string-into-a-c11-stdchrono-time-point-or-similar)
- [csv/csv.h](https://github.com/ben-strasser/fast-cpp-csv-parser)


# TODOs
- [x] Check why short trade is never entered
- [x] Check why there is bad memory access error sometimes
- [x] Implement TQDM
- [x] Add dataset downloading facility
  - [x] Install requests package
- [x] Add a config Cli
  - [x] Don't show default for update 
  - [x] Change setConfig
  - [x] add timestamp metadata
  - [x] Fixed remove parentVersion mapping
  - [x] add table formatting
  - [x] remove show command and include in list command version
  - [x] show default version in a different color
- [ ] Add dataset metadata to config command
- [ ] Add datasetinfo validate
- [ ] Check fraction valid datapoints (volume != 0.0)
- [x] Refactor out the cli/database