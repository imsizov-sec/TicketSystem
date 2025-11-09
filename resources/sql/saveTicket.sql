INSERT INTO tickets (
    title,
    description,
    project_id,
    tracker_id,
    status_id,
    priority_id,
    assignee_id,
    watcher_id,
    creator_id,
    start_date,
    attachment
)
VALUES (
    :title,
    :description,
    :projectId,
    :trackerId,
    :statusId,
    :priorityId,
    :assigneeId,
    :watcherId,
    :creatorId,
    :startDate,
    :attachment
)
