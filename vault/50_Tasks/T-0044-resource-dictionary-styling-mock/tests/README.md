The canonical test file for T-0044 lives in the repo's CTest tree so it is built + discovered by Catch2 + ctest as part of every CI run, rather than being duplicated under this task folder. It is:

```
tests/mock_handlers/resource_dictionary_test.cpp
```

That file contains the 12 test cases (40 assertions) listed in the task's Acceptance Criteria.

The task folder still keeps this `tests/` subfolder as a stable place for any task-scoped fixtures, sample inputs, or supplemental scripts — none are needed for the mock surface, hence this README is the only thing present.
