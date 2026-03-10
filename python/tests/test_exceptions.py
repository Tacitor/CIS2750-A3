import unittest
from treasure_runner.models.exceptions import status_to_exception, status_to_status_exception, InvalidArgumentError, StatusNullPointerError

class TestExceptions(unittest.TestCase):

    def setUp(self):
        pass

    def test_status_to_exception_invalid_arg(self):
        with self.assertRaises(InvalidArgumentError):
            raise status_to_exception(1)
        
    def test_status_to_exception_null_ptr(self):
        with self.assertRaises(StatusNullPointerError):
            raise status_to_status_exception(2)

if __name__ == "__main__":
    unittest.main()
