#include "LibreRTOS.h"
#include "OSlist.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(OSlist)

BOOST_AUTO_TEST_CASE(initialize_list_head) {
  struct taskHeadList_t list;

  OS_listHeadInit(&list);

  /* list */
  BOOST_CHECK_EQUAL(list.Head, (struct taskListNode_t *)&list);
  BOOST_CHECK_EQUAL(list.Tail, (struct taskListNode_t *)&list);
  BOOST_CHECK_EQUAL(list.Length, 0);
}

BOOST_AUTO_TEST_CASE(initialize_list_node) {
  struct task_t task;
  struct taskListNode_t node;

  OS_listNodeInit(&node, &task);

  /* node */
  BOOST_CHECK_EQUAL(node.Next, (struct taskListNode_t *)NULL);
  BOOST_CHECK_EQUAL(node.Previous, (struct taskListNode_t *)NULL);
  BOOST_CHECK_EQUAL(node.Value, 0);
  BOOST_CHECK_EQUAL(node.List, (struct taskHeadList_t *)NULL);
  BOOST_CHECK_EQUAL(node.Owner, &task);
}

BOOST_AUTO_TEST_CASE(insert_and_remove_node_from_list) {
  struct taskHeadList_t list;

  OS_listHeadInit(&list);

  struct task_t task;
  struct taskListNode_t node;

  OS_listNodeInit(&node, &task);

  OS_listInsertAfter(&list, list.Head, &node);

  /* node */
  BOOST_CHECK_EQUAL(node.Next, (struct taskListNode_t *)&list);
  BOOST_CHECK_EQUAL(node.Previous, (struct taskListNode_t *)&list);
  BOOST_CHECK_EQUAL(node.Value, 0);
  BOOST_CHECK_EQUAL(node.List, &list);
  BOOST_CHECK_EQUAL(node.Owner, &task);

  /* list */
  BOOST_CHECK_EQUAL(list.Head, &node);
  BOOST_CHECK_EQUAL(list.Tail, &node);
  BOOST_CHECK_EQUAL(list.Length, 1);

  OS_listRemove(&node);

  /* node */
  BOOST_CHECK_EQUAL(node.Next, (struct taskListNode_t *)NULL);
  BOOST_CHECK_EQUAL(node.Previous, (struct taskListNode_t *)NULL);
  BOOST_CHECK_EQUAL(node.Value, 0);
  BOOST_CHECK_EQUAL(node.List, (struct taskHeadList_t *)NULL);
  BOOST_CHECK_EQUAL(node.Owner, &task);

  /* list */
  BOOST_CHECK_EQUAL(list.Head, (struct taskListNode_t *)&list);
  BOOST_CHECK_EQUAL(list.Tail, (struct taskListNode_t *)&list);
  BOOST_CHECK_EQUAL(list.Length, 0);
}

BOOST_AUTO_TEST_CASE(insert_and_remove_nodes_from_list) {
  struct taskHeadList_t list;

  OS_listHeadInit(&list);

  struct task_t task1, task2, task3;
  struct taskListNode_t node1, node2, node3;

  OS_listNodeInit(&node1, &task1);
  OS_listNodeInit(&node2, &task2);
  OS_listNodeInit(&node3, &task3);

  OS_listInsertAfter(&list, list.Head, &node1);
  OS_listInsertAfter(&list, list.Head, &node3);
  OS_listInsertAfter(&list, list.Tail->Previous, &node2);

  /* node */
  BOOST_CHECK_EQUAL(node1.Next, &node2);
  BOOST_CHECK_EQUAL(node1.Previous, (struct taskListNode_t *)&list);
  BOOST_CHECK_EQUAL(node1.Value, 0);
  BOOST_CHECK_EQUAL(node1.List, &list);
  BOOST_CHECK_EQUAL(node1.Owner, &task1);

  /* node2 */
  BOOST_CHECK_EQUAL(node2.Next, &node3);
  BOOST_CHECK_EQUAL(node2.Previous, &node1);
  BOOST_CHECK_EQUAL(node2.Value, 0);
  BOOST_CHECK_EQUAL(node2.List, &list);
  BOOST_CHECK_EQUAL(node2.Owner, &task2);

  /* node3 */
  BOOST_CHECK_EQUAL(node3.Next, (struct taskListNode_t *)&list);
  BOOST_CHECK_EQUAL(node3.Previous, &node2);
  BOOST_CHECK_EQUAL(node3.Value, 0);
  BOOST_CHECK_EQUAL(node3.List, &list);
  BOOST_CHECK_EQUAL(node3.Owner, &task3);

  /* list */
  BOOST_CHECK_EQUAL(list.Head, &node1);
  BOOST_CHECK_EQUAL(list.Tail, &node3);
  BOOST_CHECK_EQUAL(list.Length, 3);

  OS_listRemove(&node2);
  OS_listRemove(&node3);
  OS_listRemove(&node1);

  /* node1 */
  BOOST_CHECK_EQUAL(node1.Next, (struct taskListNode_t *)NULL);
  BOOST_CHECK_EQUAL(node1.Previous, (struct taskListNode_t *)NULL);
  BOOST_CHECK_EQUAL(node1.Value, 0);
  BOOST_CHECK_EQUAL(node1.List, (struct taskHeadList_t *)NULL);
  BOOST_CHECK_EQUAL(node1.Owner, &task1);

  /* node2 */
  BOOST_CHECK_EQUAL(node2.Next, (struct taskListNode_t *)NULL);
  BOOST_CHECK_EQUAL(node2.Previous, (struct taskListNode_t *)NULL);
  BOOST_CHECK_EQUAL(node2.Value, 0);
  BOOST_CHECK_EQUAL(node2.List, (struct taskHeadList_t *)NULL);
  BOOST_CHECK_EQUAL(node2.Owner, &task2);

  /* node3 */
  BOOST_CHECK_EQUAL(node3.Next, (struct taskListNode_t *)NULL);
  BOOST_CHECK_EQUAL(node3.Previous, (struct taskListNode_t *)NULL);
  BOOST_CHECK_EQUAL(node3.Value, 0);
  BOOST_CHECK_EQUAL(node3.List, (struct taskHeadList_t *)NULL);
  BOOST_CHECK_EQUAL(node3.Owner, &task3);

  /* list */
  BOOST_CHECK_EQUAL(list.Head, (struct taskListNode_t *)&list);
  BOOST_CHECK_EQUAL(list.Tail, (struct taskListNode_t *)&list);
  BOOST_CHECK_EQUAL(list.Length, 0);
}

BOOST_AUTO_TEST_CASE(insert_prioritized_nodes_to_list) {
  struct taskHeadList_t list;

  OS_listHeadInit(&list);

  struct task_t task1, task2, task3;
  struct taskListNode_t node1, node2, node3;
  tick_t value1 = 1, value2 = 2, value3 = 3;

  OS_listNodeInit(&node1, &task1);
  OS_listNodeInit(&node2, &task2);
  OS_listNodeInit(&node3, &task3);

  OS_listInsert(&list, &node1, value1);
  OS_listInsert(&list, &node3, value3);
  OS_listInsert(&list, &node2, value2);

  /* node */
  BOOST_CHECK_EQUAL(node1.Next, &node2);
  BOOST_CHECK_EQUAL(node1.Previous, (struct taskListNode_t *)&list);
  BOOST_CHECK_EQUAL(node1.Value, value1);
  BOOST_CHECK_EQUAL(node1.List, &list);
  BOOST_CHECK_EQUAL(node1.Owner, &task1);

  /* node2 */
  BOOST_CHECK_EQUAL(node2.Next, &node3);
  BOOST_CHECK_EQUAL(node2.Previous, &node1);
  BOOST_CHECK_EQUAL(node2.Value, value2);
  BOOST_CHECK_EQUAL(node2.List, &list);
  BOOST_CHECK_EQUAL(node2.Owner, &task2);

  /* node3 */
  BOOST_CHECK_EQUAL(node3.Next, (struct taskListNode_t *)&list);
  BOOST_CHECK_EQUAL(node3.Previous, &node2);
  BOOST_CHECK_EQUAL(node3.Value, value3);
  BOOST_CHECK_EQUAL(node3.List, &list);
  BOOST_CHECK_EQUAL(node3.Owner, &task3);

  /* list */
  BOOST_CHECK_EQUAL(list.Head, &node1);
  BOOST_CHECK_EQUAL(list.Tail, &node3);
  BOOST_CHECK_EQUAL(list.Length, 3);
}

BOOST_AUTO_TEST_SUITE_END()
