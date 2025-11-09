INSERT INTO ticket_history (
    ticket_id,
    user_id,
    changes_summary,
    comment,
    changed_at
)
VALUES (
    :ticketId,
    :userId,
    :summary,
    :comment,
    :changedAt
)
