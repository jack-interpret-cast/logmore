import logging

logging.basicConfig(level=logging.DEBUG, format='%(asctime)s [%(levelname)s] - %(message)s')

with open("../test.log") as file:
    for line in file:
        logging.debug(line.rstrip())
        logging.info(line.rstrip())
        logging.warning(line.rstrip())
        logging.error(line.rstrip())
