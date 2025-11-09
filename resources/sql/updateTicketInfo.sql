UPDATE tickets
SET
    title = :title,
    status_id = :statusId,
    priority_id = :priorityId,
    assignee_id = :assigneeId,
    watcher_id = :watcherId,
    tracker_id = :trackerId
WHERE
    id = :ticketId;
